#include "Engine/Networking/Server.hpp"
#include "Engine/Networking/UDPEndpoint.hpp"
#include "Engine/Networking/Client.hpp"

Server::Server(IPv4Address const& address)
	: m_bindingAddr(address)
{
	m_endpoint = new UDPEndpoint(m_bindingAddr);
}


Server::~Server()
{

}


void Server::Startup()
{
	m_endpoint->Startup();
}


void Server::Shutdown()
{
	m_endpoint->Shutdown();
	delete m_endpoint;
	m_endpoint = nullptr;

	for (int i = 0; i < m_clients.size(); i++)
	{
		m_clients[i]->Shutdown();
		delete m_clients[i];
		m_clients[i] = nullptr;
	}
	m_clients.clear();
}


void Server::SendDatagram(UDPDatagram* datagram)
{
	m_endpoint->Send(datagram);
}


void Server::AddClient(Client* client)
{
	m_clients.push_back(client);
}


std::vector<Client*> const& Server::GetClients() const
{
	return m_clients;
}


IPv4Address Server::GetBindingAddr() const
{
	return m_bindingAddr;
}
