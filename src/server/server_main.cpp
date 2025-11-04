#include "echo_server_center.h"
#include <iostream>
#include <csignal>
#include <atomic>

static std::atomic<bool> g_running{true};
static EchoServerCenter* g_center = nullptr;

void signal_handler(int sig) {
    std::cout << "\nReceived signal " << sig << ", shutting down..." << std::endl;
    g_running = false;
    if (g_center) {
        g_center->Stop();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Echo Server Starting..." << std::endl;
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建服务器
    EchoServerCenter center;
    g_center = &center;
    
    // 监听端口，默认8888
    uint16_t port = 8888;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }
    
    if (!center.Listen(nullptr, port)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    // 运行事件循环
    center.Run();
    
    std::cout << "Echo Server Stopped" << std::endl;
    return 0;
}
