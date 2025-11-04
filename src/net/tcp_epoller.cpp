#include "../include/net/tcp_epoller.h"
#include "../include/common/packet.h"
#include "../include/common/packet_header.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>

TcpEpoller::TcpEpoller() : Epoller(), want_out_(false), read_state_(READING_HEADER), pending_data_read_(0) {
    fd_ = -1;
}

TcpEpoller::TcpEpoller(int fd) : Epoller(), want_out_(false), read_state_(READING_HEADER), pending_data_read_(0) {
    fd_ = fd;
}

TcpEpoller::~TcpEpoller() = default;

void TcpEpoller::ResetReadState() {
    read_state_ = READING_HEADER;
    pending_data_.clear();
    pending_data_read_ = 0;
}

void TcpEpoller::In() {
    if (fd_ < 0) {
        return;
    }
    
    std::cout << "TcpEpoller::In() called on fd: " << fd_ << ", state=" << (read_state_ == READING_HEADER ? "HEADER" : "DATA") << std::endl;
    
    // 根据状态读取包头或数据
    if (read_state_ == READING_HEADER) {
        // 读取包头
        PacketHeader header;
        size_t header_read = 0;
        uint8_t* header_buf = reinterpret_cast<uint8_t*>(&header);
        
        while (header_read < sizeof(header)) {
            ssize_t n = recv(fd_, header_buf + header_read, sizeof(header) - header_read, 0);
            
            if (n <= 0) {
                if (n == 0) {
                    std::cout << "Connection closed by peer on fd: " << fd_ << std::endl;
                    Close();
                    return;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::cout << "Read header would block, waiting for more data (got " << header_read << "/" << sizeof(header) << " bytes)" << std::endl;
                    return;
                } else {
                    std::cerr << "Read header error on fd " << fd_ << ": " << strerror(errno) << std::endl;
                    Close();
                    return;
                }
            }
            
            header_read += n;
        }
        
        std::cout << "Read header: command=" << header.command << ", length=" << header.length << std::endl;
        
        // 保存包头，准备读取数据
        pending_header_ = header;
        if (header.length > 0) {
            pending_data_.resize(header.length);
            pending_data_read_ = 0;
            read_state_ = READING_DATA;
        } else {
            // 空数据包，直接处理
            Data empty_data;
            Packet packet(header, std::move(empty_data));
            RecvImpl(std::move(packet));
            ResetReadState();
            Out();
            return;
        }
    }
    
    // 读取数据
    if (read_state_ == READING_DATA) {
        while (pending_data_read_ < pending_data_.size()) {
            ssize_t n = recv(fd_, pending_data_.data() + pending_data_read_, 
                           pending_data_.size() - pending_data_read_, 0);
            
            if (n <= 0) {
                if (n == 0) {
                    std::cout << "Connection closed while reading data on fd: " << fd_ << std::endl;
                    Close();
                    return;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::cout << "Read data would block, waiting for more data (got " << pending_data_read_ << "/" << pending_data_.size() << " bytes)" << std::endl;
                    return;
                } else {
                    std::cerr << "Read data error on fd " << fd_ << ": " << strerror(errno) << std::endl;
                    Close();
                    return;
                }
            }
            
            pending_data_read_ += n;
        }
        
        std::cout << "Read complete data: " << pending_data_read_ << " bytes" << std::endl;
        
        // 创建Packet并调用RecvImpl
        Data data(pending_data_.data(), pending_data_.size());
        Packet packet(pending_header_, std::move(data));
        RecvImpl(std::move(packet));
        
        // 重置状态，准备读取下一个包
        ResetReadState();
        
        // 尝试立即发送
        Out();
    }
}

void TcpEpoller::Out() {
    if (fd_ < 0 || send_queue_.empty()) {
        want_out_ = false;
        return;
    }
    
    std::cout << "TcpEpoller::Out() called on fd: " << fd_ << ", queue size: " << send_queue_.size() << std::endl;
    
    while (!send_queue_.empty()) {
        Packet& packet = send_queue_.front();
        
        // 发送包头
        ssize_t n = send(fd_, &packet.header(), sizeof(packet.header()), MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 暂时无法发送，保持want_out_
                return;
            }
            std::cerr << "Send header error on fd " << fd_ << ": " << strerror(errno) << std::endl;
            Close();
            return;
        }
        
        if (static_cast<size_t>(n) < sizeof(packet.header())) {
            // 部分发送，保持want_out_
            return;
        }
        
        // 发送数据
        if (packet.data().length() > 0) {
            n = send(fd_, packet.data().ptr(), packet.data().length(), MSG_NOSIGNAL);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return;
                }
                std::cerr << "Send data error on fd " << fd_ << ": " << strerror(errno) << std::endl;
                Close();
                return;
            }
            
            if (static_cast<size_t>(n) < packet.data().length()) {
                // 部分发送，保持want_out_
                return;
            }
        }
        
        // 成功发送一个完整的包
        send_queue_.pop();
    }
    
    // 队列为空
    want_out_ = false;
    
    // 调用AllSended回调
    AllSendedImpl();
}

void TcpEpoller::Send(Packet packet) {
    // 将 Packet 加入发送队列
    std::cout << "TcpEpoller::Send() called on fd: " << fd_ << std::endl;
    send_queue_.push(std::move(packet));
    want_out_ = true;
}

void TcpEpoller::Close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    want_out_ = false;
    ResetReadState();
    
    // 清空发送队列
    while (!send_queue_.empty()) {
        send_queue_.pop();
    }
}

