#pragma once
#include <string>
#include "Engine/Networking/IPv4Address.hpp"
#include<array>

constexpr int DATAGRAM_BUFFER_MAX_SIZE = 1024;

struct UDPDatagram
{
public:
	std::array<unsigned char, DATAGRAM_BUFFER_MAX_SIZE> m_buffer = {};
	IPv4Address m_destination;
	IPv4Address m_source;
	std::string m_debugString;
};

