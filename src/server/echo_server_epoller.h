#pragma once

#include "../include/net/auto_flag_tcp_epoller.h"
#include "../include/common/packet.h"

// EchoServerEpoller 类定义
// 派生自 AutoFlagTcpEpoller，实现回射逻辑

class EchoServerEpoller : public AutoFlagTcpEpoller {
public:
    EchoServerEpoller();
    explicit EchoServerEpoller(int fd);
    virtual ~EchoServerEpoller();
    
    virtual void StartImpl() override;
    virtual void RecvImpl(Packet packet) override;
    virtual void AllSendedImpl() override;
};

