#pragma once
#include <thread>
#include "Engine/Core/SynchronizedBlockingQueue.hpp"
#include "Engine/Core/SynchronizedQueue.hpp"
#include <optional>
#include "Engine/Networking/IPv4Address.hpp"
//#include "Engine/Networking/UDPDatagram.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include <string>

struct UDPDatagram;

class UDPEndpoint
{
public:
	UDPEndpoint(IPv4Address const& socketAddr);
	void BindSocket(IPv4Address const& socketAddr);
	void Startup();
	void Shutdown();

	void Send(UDPDatagram* message);
	std::optional<UDPDatagram*> Receive();

	IPv4Address GetSocketAddr() const;
	std::string GetDescription() const;

private:
	void SendProcess();
	void ReceiveProcess();

private:
	std::thread m_senderThread;
	std::thread m_receiverThread;
	SynchronizedQueue<UDPDatagram*> m_inboundQueue;
	SynchronizedBlockingQueue<UDPDatagram*> m_outboundQueue;
	UDPSocket m_socket;
	IPv4Address m_socketAddr;
};
