#include "echo_server_center.h"
#include "echo_server_epoller.h"
#include "../include/net/epoller.h"
#include <memory>

EchoServerCenter::EchoServerCenter() : EpollCenter() {}

EchoServerCenter::~EchoServerCenter() = default;

std::unique_ptr<Epoller> EchoServerCenter::NewConnectionEpoller(int fd) {
    return std::make_unique<EchoServerEpoller>(fd);
}
