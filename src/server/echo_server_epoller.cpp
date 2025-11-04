#include "echo_server_epoller.h"
#include "../include/common/packet_header.h"

EchoServerEpoller::EchoServerEpoller() : AutoFlagTcpEpoller() {}

EchoServerEpoller::EchoServerEpoller(int fd) : AutoFlagTcpEpoller(fd) {}

EchoServerEpoller::~EchoServerEpoller() = default;

void EchoServerEpoller::StartImpl() {
    // 回射无需握手，可为空
}

void EchoServerEpoller::RecvImpl(Packet packet) {
    // 回射逻辑：将收到的 packet 原样 Send
    // 或构造 DEFAULT 命令的 Packet
    if (packet.header().command == static_cast<uint32_t>(PacketHeaderCommand::DEFAULT)) {
        // 直接原样回射
        Send(std::move(packet));
    } else {
        // 处理其他命令（如 READ_EOF/WRITE_CLOSED）交给基类
    }
}

void EchoServerEpoller::AllSendedImpl() {
    // 回射是流式，通常不主动关闭
    // 除非协议约定
}

