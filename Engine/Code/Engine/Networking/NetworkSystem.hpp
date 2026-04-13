#pragma once
#include <string>
#include <optional>

class UDPEndpoint;
struct UDPDatagram;
struct IPv4Address;

struct NetworkConfig
{

};


class NetworkSystem
{
public:
	static UDPDatagram* CreateDatagram(IPv4Address const& senderAddr, IPv4Address const& destAddr);
	static void DeleteDatagram(UDPDatagram*& message);

public:
	NetworkSystem(NetworkConfig const& config);
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	UDPEndpoint* CreateUPDEndpoint(IPv4Address const& socketAddr);
	void DestroyUDPEndpoint(UDPEndpoint*& endpointToDestroy);

private:
	NetworkConfig m_config;
};
