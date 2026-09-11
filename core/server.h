#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include <atomic>

class ThreadPool;
class EpollManager;

class WebServer {
public:
    WebServer(int port, const std::string& rootDir);
    ~WebServer();

    bool start();  // 启动
    void stop();  // 停止
    void run();  // 主循环

private:
    void handleClient(int clientFd);  // 处理客户端请求
    void acceptConnections();  // 接受新连接

    // 成员变量
    int m_port;
    std::string m_rootDir;  // 根目录路径
    int m_serverFd;  // 监听socket
    std::unique_ptr<EpollManager> m_epollMgr;  // epoll管理器
    std::unique_ptr<ThreadPool> m_pool;  // 线程池
    std::atomic<bool> m_isRunning;  // 运行状态
};

#endif