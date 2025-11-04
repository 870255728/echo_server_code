#pragma once

#include "center.h"

class Epoller;

// EpollCenter 类定义
// 基于 Epoll 的 Center 实现

class EpollCenter : public Center {
public:
    EpollCenter();
    virtual ~EpollCenter();
    
protected:
    virtual std::unique_ptr<Epoller> NewConnectionEpoller(int fd) = 0;
};

