#include "server.h"
#include "../net/epoll_manager.h"
#include "../pool/threadpool.h"
#include "../http/http_request.h"
#include "../http/http_response.h"
#include "../log/log.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>

#define MAX_EVENTS 1024

WebServer::WebServer(int port, const std::string& rootDir)
    : m_port(port), m_rootDir(rootDir), m_serverFd(-1), m_isRunning(false) {
    m_epollMgr = std::make_unique<EpollManager>();
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    // 1. 创建 socket
    m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverFd == -1) {
        logMessage("socket 创建失败", LOG_ERROR);
        return false;
    }

    // 2. 端口复用
    int opt = 1;
    setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. 绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    if (bind(m_serverFd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        logMessage("bind 失败，端口 " + std::to_string(m_port) + " 可能被占用", LOG_ERROR);
        close(m_serverFd);
        return false;
    }

    // 4. 监听
    if (listen(m_serverFd, 128) == -1) {
        logMessage("listen 失败", LOG_ERROR);
        close(m_serverFd);
        return false;
    }

    // 5. 设置非阻塞
    EpollManager::setNonBlocking(m_serverFd);

    // 6. 创建 Epoll
    if (!m_epollMgr->create()) {
        logMessage("epoll 创建失败", LOG_ERROR);
        close(m_serverFd);
        return false;
    }

    // 7. 将监听 socket 加入 epoll
    if (!m_epollMgr->addFd(m_serverFd, EPOLLIN | EPOLLET)) {
        logMessage("添加监听 socket 到 epoll 失败", LOG_ERROR);
        close(m_serverFd);
        return false;
    }

    // 8. 创建线程池
    m_pool = std::make_unique<ThreadPool>(4);
    logMessage("线程池创建成功，4个工作线程");

    m_isRunning = true;
    logMessage("Server started on port " + std::to_string(m_port) + " (Linux + Epoll ET模式)");
    std::cout << "Server started on port " << m_port << " (Epoll ET)" << std::endl;

    return true;
}

void WebServer::stop() {
    m_isRunning = false;
    if (m_pool) {
        m_pool->stop();
    }
    if (m_epollMgr) {
        m_epollMgr->close();
    }
    if (m_serverFd != -1) {
        close(m_serverFd);
        m_serverFd = -1;
    }
    logMessage("Server stopped");
}

void WebServer::run() {
    if (!m_isRunning) {
        logMessage("Server 未运行，无法接受连接", LOG_ERROR);
        return;
    }

    struct epoll_event events[MAX_EVENTS];

    while (m_isRunning) {
        int nfds = m_epollMgr->wait(events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (m_isRunning) {
                logMessage("epoll_wait 失败", LOG_ERROR);
            }
            continue;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            if (fd == m_serverFd) {
                // 新连接到来
                acceptConnections();
            } else {
                // 客户端数据可读
                if (events[i].events & (EPOLLIN | EPOLLRDHUP)) {
                    // 从 epoll 中移除，防止其他线程重复处理
                    m_epollMgr->delFd(fd);
                    m_pool->enqueue([this, fd]() {
                        handleClient(fd);
                        close(fd);
                        logMessage("客户端断开，fd=" + std::to_string(fd));
                    });
                }
            }
        }
    }
}

void WebServer::acceptConnections() {
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        int clientFd = accept(m_serverFd, (struct sockaddr*)&clientAddr, &len);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // 所有新连接已处理完毕
            }
            logMessage("accept 失败", LOG_ERROR);
            break;
        }

        EpollManager::setNonBlocking(clientFd);
        m_epollMgr->addFd(clientFd, EPOLLIN | EPOLLET | EPOLLRDHUP);
        logMessage("新客户端连接，fd=" + std::to_string(clientFd));
    }
}

void WebServer::handleClient(int clientFd) {
    char buffer[4096] = {0};
    int n = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        return;
    }

    // 解析 HTTP 请求
    HttpRequest req;
    if (!req.parse(std::string(buffer, n))) {
        return;
    }

    logMessage("Request: " + req.method + " " + req.path);

    // 构造响应
    HttpResponse resp = HttpResponse::fromFile(req.path, m_rootDir);
    std::string response = resp.build();

    send(clientFd, response.c_str(), response.size(), 0);
    
    logMessage("Response: " + std::to_string(resp.statusCode) + " for " + req.path,
               resp.statusCode == 200 ? LOG_INFO : LOG_WARNING);
}