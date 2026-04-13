#include "Engine/Networking/IPv4Address.hpp"
#include "Engine/Core/StringUtils.hpp"

IPv4Address::IPv4Address(uint16_t port, uint32_t address)
	: m_portNumber(port)
	, m_networkAddress(address)
{
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.S_un.S_addr = htonl(m_networkAddress);
	m_sockaddr.sin_port = htons(m_portNumber);
}


IPv4Address::IPv4Address(uint16_t port, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
	: m_portNumber(port)
	, m_networkAddress(b1 << 24 | b2 << 16 | b3 << 8 | b4)
{
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.S_un.S_addr = htonl(m_networkAddress);
	m_sockaddr.sin_port = htons(m_portNumber);
}


IPv4Address::IPv4Address()
{

}


std::string IPv4Address::GetAsString() const
{
	uint8_t mask = 1;
	uint8_t b1 = m_networkAddress >> 24;
	uint8_t b2 = (m_networkAddress >> 16) & mask;
	uint8_t b3 = (m_networkAddress >> 8) & mask;
	uint8_t b4 = m_networkAddress & mask;
	return Stringf("%i.%i.%i.%i::%i", b1, b2, b3, b4, m_portNumber);
}


bool IPv4Address::operator!=(IPv4Address const& compare)
{
	return !(*this == compare);
}


bool IPv4Address::operator==(IPv4Address const& compare)
{
	return this->m_networkAddress == compare.m_networkAddress && this->m_portNumber == compare.m_portNumber;
}


void IPv4Address::SetNetworkAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
	m_networkAddress = b1 << 24 | b2 << 16 | b3 << 8 | b4;
}