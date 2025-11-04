#include <iostream>
#include "../include/common/packet_header.h"
#include "../include/common/packet.h"
#include "../include/common/data.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "Echo Client Starting..." << std::endl;
    
    const char* host = "127.0.0.1";
    uint16_t port = 8888;
    
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = static_cast<uint16_t>(std::atoi(argv[2]));
    }
    
    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return 1;
    }
    
    // 连接服务器
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << std::endl;
        close(sock);
        return 1;
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect: " << strerror(errno) << std::endl;
        close(sock);
        return 1;
    }
    
    std::cout << "Connected to " << host << ":" << port << std::endl;
    
    // 发送几个测试包
    for (int i = 1; i <= 3; i++) {
        // 构造测试数据
        std::string test_msg = "Hello from client, message #" + std::to_string(i);
        
        // 创建包头
        PacketHeader header{};
        header.command = static_cast<uint32_t>(PacketHeaderCommand::DEFAULT);
        header.length = test_msg.length();
        header.error = 0;
        header.extra1 = i;
        header.extra2 = 0;
        
        // 创建数据
        Data data(test_msg.c_str(), test_msg.length());
        
        // 发送包头
        ssize_t n = send(sock, &header, sizeof(header), 0);
        if (n != sizeof(header)) {
            std::cerr << "Failed to send header: " << strerror(errno) << std::endl;
            break;
        }
        
        // 发送数据
        if (data.length() > 0) {
            n = send(sock, data.ptr(), data.length(), 0);
            if (n != static_cast<ssize_t>(data.length())) {
                std::cerr << "Failed to send data: " << strerror(errno) << std::endl;
                break;
            }
        }
        
        std::cout << "Sent: " << test_msg << std::endl;
        
        // 接收回复
        PacketHeader recv_header{};
        n = recv(sock, &recv_header, sizeof(recv_header), MSG_WAITALL);
        if (n != sizeof(recv_header)) {
            std::cerr << "Failed to receive header: " << strerror(errno) << std::endl;
            break;
        }
        
        if (recv_header.length > 0) {
            Data recv_data(recv_header.length);
            n = recv(sock, const_cast<void*>(recv_data.ptr()), recv_header.length, MSG_WAITALL);
            if (n != static_cast<ssize_t>(recv_header.length)) {
                std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
                break;
            }
            
            std::string recv_msg(static_cast<const char*>(recv_data.ptr()), recv_data.length());
            std::cout << "Received: " << recv_msg << std::endl;
            
            // 验证数据一致性
            if (recv_msg == test_msg) {
                std::cout << "✓ Echo test #" << i << " passed!" << std::endl;
            } else {
                std::cout << "✗ Echo test #" << i << " failed! Mismatch!" << std::endl;
            }
        }
        
        // 短暂延迟
        usleep(100000); // 100ms
    }
    
    std::cout << "Echo Client Finished" << std::endl;
    close(sock);
    return 0;
}
