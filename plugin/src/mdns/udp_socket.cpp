#include "udp_socket.h"

std::vector<sockaddr_in> UdpSocket::listAddress()
{
	std::vector<sockaddr_in> addresses;
#ifdef _WIN32
	IP_ADAPTER_ADDRESSES *adapter_address = 0;
	ULONG address_size = 8000;
	unsigned int ret;
	unsigned int num_retries = 4;
	do
	{
		adapter_address = (IP_ADAPTER_ADDRESSES *)malloc(address_size);
		ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0,
								   adapter_address, &address_size);
		if (ret == ERROR_BUFFER_OVERFLOW)
		{
			free(adapter_address);
			adapter_address = 0;
			address_size *= 2;
		}
		else
		{
			break;
		}
	} while (num_retries-- > 0);

	if (!adapter_address || (ret != NO_ERROR))
	{
		free(adapter_address);
		printf("Failed to get network adapter addresses\n");
		return addresses;
	}
	for (PIP_ADAPTER_ADDRESSES adapter = adapter_address; adapter; adapter = adapter->Next)
	{
		if (adapter->TunnelType == TUNNEL_TYPE_TEREDO)
			continue;
		if (adapter->OperStatus != IfOperStatusUp)
			continue;

		for (IP_ADAPTER_UNICAST_ADDRESS *unicast = adapter->FirstUnicastAddress; unicast;
			 unicast = unicast->Next)
		{
			if (unicast->Address.lpSockaddr->sa_family == AF_INET)
			{
				struct sockaddr_in *saddr = (struct sockaddr_in *)unicast->Address.lpSockaddr;
				if (saddr->sin_addr.S_un.S_addr == inet_addr("127.0.0.1"))
					continue;

				addresses.push_back(*saddr);
				std::cout << (int)saddr->sin_addr.S_un.S_un_b.s_b1 << "."
						  << (int)saddr->sin_addr.S_un.S_un_b.s_b2 << "."
						  << (int)saddr->sin_addr.S_un.S_un_b.s_b3 << "."
						  << (int)saddr->sin_addr.S_un.S_un_b.s_b4 << std::endl;
			}
		}
	}

	free(adapter_address);

#else
	struct ifaddrs *interfaces = NULL;
	struct ifaddrs *temp_addr = NULL;

	sockaddr_in loopback;
	inet_pton(AF_INET, "127.0.0.1", &(loopback.sin_addr));

	int success = 0;
	// retrieve the current interfaces - returns 0 on success
	success = getifaddrs(&interfaces);
	if (success == 0)
	{
		// Loop through linked list of interfaces
		temp_addr = interfaces;
		while (temp_addr != NULL)
		{
			if (temp_addr->ifa_addr->sa_family == AF_INET && ((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr.s_addr != loopback.sin_addr.s_addr)
			{
				addresses.push_back(*((struct sockaddr_in *)temp_addr->ifa_addr));
			}
			temp_addr = temp_addr->ifa_next;
		}
	}
	// Free memory
	freeifaddrs(interfaces);

#endif

	return addresses;
}

UdpSocket::UdpSocket(sockaddr_in addr)
{
	sock = mdns_socket_open_ipv4(&addr);
	this->addr = addr;
}

UdpSocket::~UdpSocket()
{
	mdns_socket_close(sock);
}

int UdpSocket::getSock()
{
	return sock;
}

sockaddr_in UdpSocket::getAddr()
{
	return addr;
}
