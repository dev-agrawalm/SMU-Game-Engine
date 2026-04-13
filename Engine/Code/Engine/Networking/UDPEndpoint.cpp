#include "Engine/Networking/UDPEndpoint.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Networking/UDPDatagram.hpp"

void UDPEndpoint::Startup()
{
	BindSocket(m_socketAddr);
	m_senderThread = std::thread(&UDPEndpoint::SendProcess, this);
	m_receiverThread = std::thread(&UDPEndpoint::ReceiveProcess, this);
}


void UDPEndpoint::Send(UDPDatagram* message)
{
	m_outboundQueue.Push(message);
}


std::optional<UDPDatagram*> UDPEndpoint::Receive()
{
	return m_inboundQueue.Pop();
}


void UDPEndpoint::Shutdown()
{
	m_outboundQueue.Exit();
	m_senderThread.join();
	m_socket.Close();
	m_receiverThread.join();
}


UDPEndpoint::UDPEndpoint(IPv4Address const& socketAddr)
	: m_socketAddr(socketAddr)
{
}


void UDPEndpoint::BindSocket(IPv4Address const& socketAddr)
{
	if (m_socket.IsOpen())
	{
		m_socket.Close();
	}
	m_socket.Open();
	m_socket.Bind(socketAddr);
}


IPv4Address UDPEndpoint::GetSocketAddr() const
{
	return m_socket.m_address;
}


std::string UDPEndpoint::GetDescription() const
{
	return Stringf("IPv4 addr: %i  Port No: %i", m_socket.m_address.m_networkAddress, m_socket.m_address.m_portNumber);
}


void UDPEndpoint::SendProcess()
{
	std::optional<UDPDatagram*> message;
	while (true)
	{
		message = m_outboundQueue.Pop();
		if (message == std::nullopt)
		{
			break;
		}
		else
		{
			UDPDatagram* messagePtr = message.value();
			m_socket.Send(messagePtr);
		}
	}
}


void UDPEndpoint::ReceiveProcess()
{
	std::optional<UDPDatagram*> message;
	while (true)
	{
		message = m_socket.Receive();
		if (message == std::nullopt)
		{
			break;
		}
		m_inboundQueue.Push(message.value());
	}
}