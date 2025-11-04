#pragma once

#include "packet_header.h"
#include "data.h"

class Packet {
public:
    Packet();
    Packet(const PacketHeader& header, const Data& data);
    Packet(const PacketHeader& header, Data&& data);
    Packet(const Packet& other);
    Packet(Packet&& other) noexcept;
    ~Packet();
    
    Packet& operator=(const Packet& other);
    Packet& operator=(Packet&& other) noexcept;
    
    const PacketHeader& header() const { return header_; }
    PacketHeader& header() { return header_; }
    
    const Data& data() const { return data_; }
    Data& data() { return data_; }
    
    Packet Ack() const;
    void Send();
    
private:
    PacketHeader header_;
    Data data_;
};

