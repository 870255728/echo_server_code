#pragma once

#include "../include/core/epoll_center.h"
#include <memory>

class Epoller;

// EchoServerCenter 类定义
// 覆写 NewConnectionEpoller 返回 EchoServerEpoller

class EchoServerCenter : public EpollCenter {
public:
    EchoServerCenter();
    virtual ~EchoServerCenter();
    
protected:
    virtual std::unique_ptr<Epoller> NewConnectionEpoller(int fd) override;
};

