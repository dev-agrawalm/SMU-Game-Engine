#pragma once
#include <winsock2.h>
#include <string>


struct IPv4Address
{
public:
	IPv4Address();
	IPv4Address(uint16_t port, uint32_t address);
	IPv4Address(uint16_t port, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

	void SetNetworkAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

	std::string GetAsString() const;

	bool operator==(IPv4Address const& compare);
	bool operator!=(IPv4Address const& compare);
public:
	uint32_t m_networkAddress = 0;
	uint16_t m_portNumber = 0;
	sockaddr_in m_sockaddr = {};
};
