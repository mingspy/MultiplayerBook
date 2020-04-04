#include "RoboCatPCH.h"


string	SocketAddress::ToString() const
{
	const sockaddr_in* s = GetAsSockAddrIn();
	char destinationBuffer[ 128 ];
#if _WIN32
	InetNtop( s->sin_family, const_cast< in_addr* >( &s->sin_addr ), destinationBuffer, sizeof( destinationBuffer ) );
	return StringUtils::Sprintf( "%s:%d", destinationBuffer, ntohs( s->sin_port ) );
#else
	//not implement on mac for now...
	char * p = inet_ntoa( s->sin_addr );
	return StringUtils::Sprintf( "%s:%d", p, ntohs( s->sin_port ) );
#endif
}

