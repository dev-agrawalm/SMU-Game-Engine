#pragma once
#include "Engine/Networking/IPv4Address.hpp"
#include <optional>

struct UDPDatagram;

struct UDPSocket
{
public:
	UDPSocket();
	void Open();
	void Bind(IPv4Address const& bindingAddress);
	void Send(UDPDatagram* message);
	std::optional<UDPDatagram*> Receive();
	void Close();

	bool IsOpen() const;

public:
	IPv4Address m_address;
	bool m_isOpen = false;

private:
	SOCKET m_socket = 0;
};
