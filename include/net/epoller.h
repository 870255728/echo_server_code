#pragma once

#include <cstdint>

class Packet;

// Epoller 基类定义

class Epoller {
public:
    Epoller();
    virtual ~Epoller();
    
    virtual void StartImpl() {}
    virtual void In() {}
    virtual void Out() {}
    virtual void RecvImpl(Packet packet) = 0;
    virtual void AllSendedImpl() {}
    
    int GetFd() const { return fd_; }
    
protected:
    int fd_;
};

