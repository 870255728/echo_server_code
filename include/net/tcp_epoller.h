#pragma once

#include "epoller.h"
#include "../common/packet.h"
#include "../common/packet_header.h"
#include <queue>
#include <memory>
#include <vector>

// TcpEpoller 类定义
// 单连接读写与发送队列管理

class TcpEpoller : public Epoller {
public:
    TcpEpoller();
    explicit TcpEpoller(int fd);
    virtual ~TcpEpoller();
    
    virtual void In() override;
    virtual void Out() override;
    void Send(Packet packet);
    void Close();
    
    virtual void RecvImpl(Packet packet) override = 0;
    
protected:
    std::queue<Packet> send_queue_;
    bool want_out_;
    
    // 读取状态
    enum ReadState {
        READING_HEADER,
        READING_DATA
    };
    ReadState read_state_;
    PacketHeader pending_header_;
    std::vector<uint8_t> pending_data_;
    size_t pending_data_read_;
    
    void ResetReadState();
};

