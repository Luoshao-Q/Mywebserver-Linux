#ifndef EPOLL_MANAGER_H
#define EPOLL_MANAGER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

// Epoll 管理类（仅负责 epoll 的创建、添加、删除、等待）
class EpollManager {
public:
    EpollManager() : m_epollFd(-1) {}
    ~EpollManager() { close(); }

    bool create() {
        m_epollFd = epoll_create1(0);
        return m_epollFd != -1;
    }

    void close() {
        if (m_epollFd != -1) {
            ::close(m_epollFd);
            m_epollFd = -1;
        }
    }

    int getFd() const { return m_epollFd; }

    bool addFd(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        return epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev) == 0;
    }

    bool delFd(int fd) {
        return epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr) == 0;
    }

    bool modFd(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        return epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &ev) == 0;
    }

    int wait(struct epoll_event* events, int maxEvents, int timeout = -1) {
        return epoll_wait(m_epollFd, events, maxEvents, timeout);
    }

    static void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

private:
    int m_epollFd;
};

#endif