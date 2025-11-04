#pragma once

#include <cstdint>


enum class PacketHeaderCommand : uint32_t {
    DEFAULT = 9,
    ACK = 1,
    ERROR = 2,
    READ_EOF = 3,
    WRITE_CLOSED = 4
};

struct PacketHeader {
    uint32_t command;
    uint32_t length;
    uint32_t error;
    uint32_t extra1;
    uint32_t extra2;
};

