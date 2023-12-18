#include "mdns_service.h"

char namebuffer[256];

int record_callback(int sock, const struct sockaddr *from, size_t addrlen,
                    mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                    uint16_t rclass, uint32_t ttl, const void *data, size_t size,
                    size_t name_offset, size_t name_length, size_t record_offset,
                    size_t record_length, void *user_data)
{
    if (entry != MDNS_ENTRYTYPE_QUESTION)
        return 0;
    auto self = (MDNSService *)user_data;

    auto domain = self->domain;

    mdns_string_t name = mdns_string_extract(data, size, &name_offset, namebuffer, sizeof(namebuffer));
    std::cout << std::string(name.str, name.length) << std::endl;

    static char sendbuffer[1024];
    if (rtype == MDNS_RECORDTYPE_A && strncmp(name.str, domain, strlen(domain)) == 0)
    {
        mdns_record_t answer;
        answer.name.length = strlen(domain);
        answer.name.str = domain;
        answer.type = MDNS_RECORDTYPE_A;
        answer.data.a.addr = self->addrForSock(sock);
        answer.rclass = 0;
        answer.ttl = 0;
        uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

        if (unicast)
        {
            mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
                                      query_id, (mdns_record_type_t)rtype, name.str, name.length, answer, 0, 0,
                                      0, 0);
        }
        else
        {
            mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, 0, 0);
        }
    }
    return 0;
}

int MDNSService::waitForReadableSocket(timeval timeout, std::function<void(UdpSocket &)> callback)
{
    int nfds = 0;
    fd_set readfs;
    FD_ZERO(&readfs);

    for (auto &sock : sockets)
    {
        if (nfds < sock->getSock() + 1)
            nfds = sock->getSock() + 1;
        FD_SET(sock->getSock(), &readfs);
    }
    auto res = select(nfds, &readfs, 0, 0, &timeout);
    if (res >= 0)
    {
        for (auto &sock : sockets)
        {
            if (FD_ISSET(sock->getSock(), &readfs))
            {
                callback(*sock);
            }
            FD_SET(sock->getSock(), &readfs);
        }
    }
    return res;
}

sockaddr_in MDNSService::addrForSock(int sock)
{
    auto result = std::find_if(sockets.begin(), sockets.end(), [&](auto &s)
                               { return s->getSock() == sock; });
    return result->get()->getAddr();
}

std::vector<std::string> MDNSService::listAddress()
{
    std::vector<std::string> result;
    for (auto &sock : sockets)
    {
        auto addr = sock->getAddr();
#if _WIN32
        std::ostringstream stream;
        addr.sin_addr.s_addr
                stream
            << (int)addr.sin_addr.S_un.S_un_b.s_b1 << "."
            << (int)addr.sin_addr.S_un.S_un_b.s_b2 << "."
            << (int)addr.sin_addr.S_un.S_un_b.s_b3 << "."
            << (int)addr.sin_addr.S_un.S_un_b.s_b4;
        result.push_back(stream.str());
#else
        result.push_back(inet_ntoa(addr.sin_addr));
#endif
    }
    return result;
}

MDNSService::MDNSService(const char *domain) : juce::Thread("MDNS_Thread"), domain(domain)
{
}

MDNSService::~MDNSService()
{
    stop();
}

void MDNSService::start()
{
#if _WIN32
    WORD versionWanted = MAKEWORD(1, 1);
    WSADATA wsaData;
    auto res = WSAStartup(versionWanted, &wsaData);
    if (res)
    {
        return;
    }
#endif

    auto addresses = UdpSocket::listAddress();
    for (auto addr : addresses)
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(MDNS_PORT);
        sockets.push_back(std::make_unique<UdpSocket>(addr));
    }
    startThread();
}

void MDNSService::run()
{
    size_t bufferSize = 2048;
    auto buffer = new char[bufferSize];
    while (!threadShouldExit())
    {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        waitForReadableSocket(timeout, [&](UdpSocket &sock)
                              { mdns_socket_listen(sock.getSock(), buffer, bufferSize, record_callback, this); });
    }
}

void MDNSService::stop()
{
    stopThread(-1);

#if _WIN32
    WSACleanup();
#endif
}
