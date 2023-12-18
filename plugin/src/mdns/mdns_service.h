#pragma once
#include <mdns.h>
#include <juce_core/juce_core.h>
#include <vector>
#include "udp_socket.h"
#include <errno.h>
#if _WIN32
    #include <WinSock2.h>
    #include <iphlpapi.h>
#endif

class MDNSService : public juce::Thread {
    std::vector<std::unique_ptr<UdpSocket>> sockets;

    int waitForReadableSocket(timeval timeout, std::function<void(UdpSocket&)> callback);
public:
    const char* domain;

    MDNSService(const char* domain);
    ~MDNSService();
    void start();
    void stop();
    void run() override;
    
    sockaddr_in addrForSock(int sock);

    std::vector<std::string> listAddress();
};