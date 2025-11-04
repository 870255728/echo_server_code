#include "../include/common/packet.h"

Packet::Packet() : header_(), data_() {}

Packet::Packet(const PacketHeader& header, const Data& data) 
    : header_(header), data_(data) {}

Packet::Packet(const PacketHeader& header, Data&& data) 
    : header_(header), data_(std::move(data)) {}

Packet::Packet(const Packet& other) 
    : header_(other.header_), data_(other.data_) {}

Packet::Packet(Packet&& other) noexcept 
    : header_(other.header_), data_(std::move(other.data_)) {}

Packet::~Packet() = default;

Packet& Packet::operator=(const Packet& other) {
    if (this != &other) {
        header_ = other.header_;
        data_ = other.data_;
    }
    return *this;
}

Packet& Packet::operator=(Packet&& other) noexcept {
    if (this != &other) {
        header_ = other.header_;
        data_ = std::move(other.data_);
    }
    return *this;
}

Packet Packet::Ack() const {
    PacketHeader ack_header = header_;
    ack_header.command = static_cast<uint32_t>(PacketHeaderCommand::ACK);
    return Packet(ack_header, data_);
}

void Packet::Send() {
}

