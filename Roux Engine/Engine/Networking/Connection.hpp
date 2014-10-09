#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#pragma once

#include <WinSock2.h>

//-----------------------------------------------------------------------------------------------
class Connection
{
public:

	Connection();
	~Connection();

	SOCKET				m_socket;
	struct sockaddr_in	m_socketAddressInfo;
	int					m_socketAddrInfoLength;
};


#endif
