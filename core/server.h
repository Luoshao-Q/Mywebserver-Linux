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

    bool start();
    void stop();
    void run();

private:
    void handleClient(int clientFd);
    void acceptConnections();

    int m_port;
    std::string m_rootDir;
    int m_serverFd;
    std::unique_ptr<EpollManager> m_epollMgr;
    std::unique_ptr<ThreadPool> m_pool;
    std::atomic<bool> m_isRunning;
};

#endif