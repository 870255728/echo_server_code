#include "../include/net/auto_flag_tcp_epoller.h"

AutoFlagTcpEpoller::AutoFlagTcpEpoller() : TcpEpoller() {}

AutoFlagTcpEpoller::AutoFlagTcpEpoller(int fd) : TcpEpoller(fd) {}

AutoFlagTcpEpoller::~AutoFlagTcpEpoller() = default;

