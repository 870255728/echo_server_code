#pragma once

#include "tcp_epoller.h"

class AutoFlagTcpEpoller : public TcpEpoller {
public:
    AutoFlagTcpEpoller();
    explicit AutoFlagTcpEpoller(int fd);
    virtual ~AutoFlagTcpEpoller();
    
    // 自动管理 WantOut 标志
    // 继承 TcpEpoller 的功能
};

