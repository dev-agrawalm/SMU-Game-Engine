#include "Engine/Networking/NetworkSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdexcept>
#include <winsock2.h>
#include "Engine/Networking/UDPEndpoint.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/UDPDatagram.hpp"

#pragma comment(lib, "Ws2_32.lib")

UDPDatagram* NetworkSystem::CreateDatagram(IPv4Address const& senderAddr, IPv4Address const& destAddr)
{
	UDPDatagram* newCreatePacket = new UDPDatagram();
	newCreatePacket->m_source = senderAddr;
	newCreatePacket->m_destination = destAddr;
	return newCreatePacket;
}


void NetworkSystem::DeleteDatagram(UDPDatagram*& message)
{
	delete message;
	message = nullptr;
}


NetworkSystem::NetworkSystem(NetworkConfig const& config)
	: m_config(config)
{

}


void NetworkSystem::Startup()
{
	WSADATA wsaData = {};
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		ERROR_AND_DIE("Failed to initialize network system");
	}
}


void NetworkSystem::Shutdown()
{
	int iResult = WSACleanup();
	if (iResult != 0)
	{
		ERROR_AND_DIE("Failed to shutdown network system");
	}
}


void NetworkSystem::BeginFrame()
{

}


void NetworkSystem::EndFrame()
{

}


UDPEndpoint* NetworkSystem::CreateUPDEndpoint(IPv4Address const& socketAddr)
{
	UDPEndpoint* endpoint = new UDPEndpoint(socketAddr);
	return endpoint;
}


void NetworkSystem::DestroyUDPEndpoint(UDPEndpoint*& endpointToDestroy)
{
	delete endpointToDestroy;
	endpointToDestroy = nullptr;
}
