#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/UDPDatagram.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Networking/NetworkSystem.hpp"

UDPSocket::UDPSocket()
{
}


void UDPSocket::Open()
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
	{
		ERROR_AND_DIE("Failed to create udp socket");
	}
}


void UDPSocket::Bind(IPv4Address const& bindingAddress)
{
	if (!m_isOpen)
	{
		Open();
	}

	int result = bind(m_socket, (SOCKADDR*) &bindingAddress.m_sockaddr, sizeof(bindingAddress.m_sockaddr));
	if (result != 0)
	{
		int error = WSAGetLastError();
		ERROR_AND_DIE(Stringf("Failed to bind socket to address. Error code %d", error));
	}

	m_address = bindingAddress;
}


void UDPSocket::Send(UDPDatagram* message)
{
	unsigned char const* messageBuffer = message->m_buffer.data();
	int messageSize = 1024;

	IPv4Address destinationAddr = message->m_destination;
	int result = sendto(m_socket, (char*) messageBuffer, messageSize, 0, (SOCKADDR*) &destinationAddr.m_sockaddr, sizeof(destinationAddr.m_sockaddr));
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		ERROR_AND_DIE(Stringf("%s: Failed to send udp message. Error code %d", message->m_debugString.c_str(), error));
	}
	NetworkSystem::DeleteDatagram(message);
}


std::optional<UDPDatagram*> UDPSocket::Receive()
{
	int messageBufferLength = 1024;
	char messageBuffer[1024] = {};

	std::optional<UDPDatagram*> receivedMessage = std::nullopt;

	sockaddr_in senderSockAddr = {};
	int senderAddrSize = sizeof(senderSockAddr);
	int result = recvfrom(m_socket, messageBuffer, messageBufferLength, 0, (SOCKADDR*) &senderSockAddr, &senderAddrSize);
	if (result == SOCKET_ERROR)
	{
		return receivedMessage;
	}
	IPv4Address senderAddr = IPv4Address(
		ntohs(senderSockAddr.sin_port),
		senderSockAddr.sin_addr.S_un.S_un_b.s_b1,
		senderSockAddr.sin_addr.S_un.S_un_b.s_b2,
		senderSockAddr.sin_addr.S_un.S_un_b.s_b3,
		senderSockAddr.sin_addr.S_un.S_un_b.s_b4
	);

	receivedMessage = NetworkSystem::CreateDatagram(senderAddr, m_address);
	receivedMessage.value()->m_source = senderAddr;
	receivedMessage.value()->m_destination = m_address;
	for (int i = 0; i < 1024; i++)
	{
		receivedMessage.value()->m_buffer[i] = messageBuffer[i];
	}
	return receivedMessage;
}


void UDPSocket::Close()
{
	int result = closesocket(m_socket);
	if (result != 0)
	{
		int error = WSAGetLastError();
		ERROR_AND_DIE(Stringf("Failed to close socket. Error code %d", error));
	}
	m_isOpen = false;
}


bool UDPSocket::IsOpen() const
{
	return m_isOpen;
}

