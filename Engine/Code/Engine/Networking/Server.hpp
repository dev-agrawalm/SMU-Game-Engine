#pragma once
#include <vector>
#include "Engine/Networking/IPv4Address.hpp"

class Client;
class UDPEndpoint;
struct UDPDatagram;

class Server
{
public:
	Server(IPv4Address const& bindingAddr);
	virtual ~Server();

	virtual void Startup();
	virtual void Shutdown();

	virtual void SendDatagram(UDPDatagram* datagram);
	virtual void AddClient(Client* client);

	std::vector<Client*> const& GetClients() const;
	IPv4Address GetBindingAddr() const;

	std::vector<Client*> m_clients;
protected:
	UDPEndpoint* m_endpoint = nullptr;
	IPv4Address m_bindingAddr;
};
