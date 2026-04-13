#include "Engine/Networking/Client.hpp"
#include "Engine/Networking/Server.hpp"

Client::Client(Server* server, IPv4Address const& clientAddr, IPv4Address const& serverAddr)
	: m_server(server)
	, m_clientAddr(clientAddr)
	, m_serverAddr(serverAddr)
{

}


Client::~Client()
{

}


void Client::Startup()
{

}


void Client::Shutdown()
{

}


IPv4Address Client::GetClientAddr() const
{
	return m_clientAddr;
}


IPv4Address Client::GetServerAddr() const
{
	return m_serverAddr;
}


Server* Client::GetServer() const
{
	return m_server;
}

