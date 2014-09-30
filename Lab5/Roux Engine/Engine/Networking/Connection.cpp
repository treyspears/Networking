#include "Connection.hpp"

//-----------------------------------------------------------------------------------------------
Connection::Connection()
	: m_socket( SOCKET_ERROR )
	, m_socketAddrInfoLength( sizeof( m_socketAddressInfo ) )
{
	memset( ( char* )&m_socketAddressInfo, 0, m_socketAddrInfoLength );
}

//-----------------------------------------------------------------------------------------------
Connection::~Connection()
{

}
