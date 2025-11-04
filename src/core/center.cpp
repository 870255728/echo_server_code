#include "../include/core/center.h"
#include "../include/net/epoller.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>

#ifndef _WIN32
#include <sys/epoll.h>
#endif

Center::Center() : listen_fd_(-1), epoll_fd_(-1), running_(false) {}

Center::~Center() {
    Stop();
}

bool Center::Listen(const char* host, uint16_t port) {
    // 创建监听socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd_ < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 设置socket选项
    int reuse = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    // 绑定地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (host == nullptr || strlen(host) == 0 || strcmp(host, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
            std::cerr << "Invalid address: " << host << std::endl;
            close(listen_fd_);
            listen_fd_ = -1;
            return false;
        }
    }
    
    if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind: " << strerror(errno) << std::endl;
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    // 开始监听
    if (listen(listen_fd_, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    // 创建epoll实例
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        std::cerr << "Failed to create epoll: " << strerror(errno) << std::endl;
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    // 将监听socket加入epoll
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
        std::cerr << "Failed to add listen fd to epoll: " << strerror(errno) << std::endl;
        close(epoll_fd_);
        close(listen_fd_);
        epoll_fd_ = -1;
        listen_fd_ = -1;
        return false;
    }
    
    std::cout << "Listening on " << (host ? host : "0.0.0.0") << ":" << port << std::endl;
    return true;
}

int Center::GetFd(const Epoller* epoller) const {
    return epoller ? epoller->GetFd() : -1;
}

void Center::AddEpoller(std::unique_ptr<Epoller> epoller) {
    int fd = GetFd(epoller.get());
    if (fd < 0) {
        return;
    }
    
    // 将fd加入epoll
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.ptr = epoller.get();
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        std::cerr << "Failed to add fd to epoll: " << strerror(errno) << std::endl;
        return;
    }
    
    epollers_[fd] = std::move(epoller);
}

void Center::RemoveEpoller(Epoller* epoller) {
    if (!epoller) {
        return;
    }
    
    int fd = epoller->GetFd();
    
    // 从epoll中移除
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    
    // 从map中移除
    epollers_.erase(fd);
    
    std::cout << "Removed epoller for fd: " << fd << std::endl;
}

void Center::Run() {
    if (listen_fd_ < 0 || epoll_fd_ < 0) {
        std::cerr << "Server not listening" << std::endl;
        return;
    }
    
    running_ = true;
    epoll_event events[MAX_EVENTS];
    
    std::cout << "Starting event loop..." << std::endl;
    
    while (running_) {
        int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        
        if (num_events < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        }
        
        for (int i = 0; i < num_events; i++) {
            uint32_t event_flags = events[i].events;
            
            // 判断是否是监听socket：监听socket使用data.fd，其他使用data.ptr
            if (events[i].data.fd == listen_fd_) {
                while (true) {
                    sockaddr_in client_addr{};
                    socklen_t addr_len = sizeof(client_addr);
                    int client_fd = accept4(listen_fd_, 
                                            (struct sockaddr*)&client_addr, 
                                            &addr_len, 
                                            SOCK_NONBLOCK);
                    
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                        break;
                    }
                    
                    // 创建新的Epoller
                    auto epoller = NewConnectionEpoller(client_fd);
                    if (epoller) {
                        AddEpoller(std::move(epoller));
                        
                        char ip_str[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
                        std::cout << "New connection from " << ip_str 
                                  << ":" << ntohs(client_addr.sin_port) 
                                  << " (fd: " << client_fd << ")" << std::endl;
                    } else {
                        close(client_fd);
                    }
                }
            } else {
                // 处理已连接socket的事件 - 使用data.ptr获取Epoller指针
                Epoller* epoller = static_cast<Epoller*>(events[i].data.ptr);
                if (epoller) {
                    int fd = epoller->GetFd();
                    
                    std::cout << "Event on fd " << fd << ": flags=" << std::hex << event_flags << std::dec << std::endl;
                    
                    // 错误或挂断
                    if (event_flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                        std::cout << "Connection closed or error on fd: " << fd << std::endl;
                        RemoveEpoller(epoller);
                        close(fd);
                        continue;
                    }
                    
                    // 读事件
                    if (event_flags & EPOLLIN) {
                        epoller->In();
                    }
                    
                    // 写事件
                    if (event_flags & EPOLLOUT) {
                        epoller->Out();
                    }
                } else {
                    std::cerr << "Epoller pointer is null" << std::endl;
                }
            }
        }
    }
    
    std::cout << "Event loop stopped" << std::endl;
}

void Center::Stop() {
    running_ = false;
    
    // 清理所有连接
    for (auto& pair : epollers_) {
        close(pair.first);
    }
    epollers_.clear();
    
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
    
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}
