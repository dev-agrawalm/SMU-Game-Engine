#pragma once
#include "Engine/Networking/IPv4Address.hpp"

class Server;

class Client
{
public:
	Client(Server* server, IPv4Address const& clientAddr, IPv4Address const& serverAddr);
	virtual ~Client();

	virtual void Startup() = 0;
	virtual void Shutdown() = 0;

	IPv4Address GetClientAddr() const;
	IPv4Address GetServerAddr() const;
	Server* GetServer() const;
protected:
	IPv4Address m_clientAddr;
	IPv4Address m_serverAddr;
	Server* m_server = nullptr;
};
