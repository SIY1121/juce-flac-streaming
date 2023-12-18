#pragma once
#include <iostream>
#include <vector>
#include <mdns.h>

#if _WIN32
#include <WinSock2.h>
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

class UdpSocket
{
    int sock = 0;
    sockaddr_in addr;

public:
    static std::vector<sockaddr_in> listAddress();

    UdpSocket(sockaddr_in);
    ~UdpSocket();
    int getSock();
    sockaddr_in getAddr();
};