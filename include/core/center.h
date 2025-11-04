#pragma once

#include <memory>
#include <cstdint>
#include <map>

#ifdef _WIN32
// Windows 下不支持epoll，我们需要做特殊处理
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/epoll.h>
#endif

class Epoller;

// Center 类定义
// 负责监听、接入连接、事件轮询与资源回收

class Center {
public:
    Center();
    virtual ~Center();
    
    bool Listen(const char* host, uint16_t port);
    void Run();
    void Stop();
    
protected:
    virtual std::unique_ptr<Epoller> NewConnectionEpoller(int fd) = 0;
    int GetFd(const Epoller* epoller) const;
    void AddEpoller(std::unique_ptr<Epoller> epoller);
    void RemoveEpoller(Epoller* epoller);
    
private:
    int listen_fd_;
    int epoll_fd_;
    bool running_;
    std::map<int, std::unique_ptr<Epoller>> epollers_;
    
    static constexpr int MAX_EVENTS = 1024;
};

