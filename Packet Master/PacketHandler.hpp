#include "Utility.hpp"
#include "PacketDefs.hpp"
#include <map>
#include "IPCHandler.hpp"

class PacketHandler
{
	SocketStruct * pSockets[ 2 ]; // FOR FUTURE USE
	const static int log_send = 0;
	const static int log_recv = 0;
	const static int log_bytes = 0;

	using handler = void( PacketHandler:: * )( PacketHeader * pPacket, int size );
	IPCHandler * pIPCHandler = nullptr;

	// Only purpose is to log a packet to file and display bytes
	void LoggingHandler( PacketHeader * pPacket, int size );

	// RECV
	void PositionHandler( PacketHeader * pPacket, int size );
	void EntityPosition( PacketHeader * pPacket, int size );
	// SEND
	void JumpHandler( PacketHeader * pPacket, int size );
	void VelocityHandler( PacketHeader * pPacket, int size );

#define HANDLER(id, fn) { id, &PacketHandler::fn },

	// TODO: Turn into array and implement own parser for spPEEED
	std::map< short, handler > pRecvHandlers =
	{
		// 0x0003 also position? Perhaps NPCS. for this one though, offset 0x7 is NOT packet size,
		//it appears to be at 0x8, wassup with that, and we need to jump 0x15 bytes to get to the start of the data. WHAAAT
		// HEY! It appears, almost EVERY packet has our position in it, WHAAAAAAAAAT?
		HANDLER( 0x4003, PositionHandler )
		//HANDLER( 0x0003, LoggingHandler )
		//HANDLER( 0x0003, EntityPosition )
		//HANDLER( 0x4003, LoggingHandler )
		//HANDLER( 0x5200, LoggingHandler )
		//HANDLER( 0x3C00, LoggingHandler )

	};

	std::map< short, handler > pSendHandlers =
	{
		//HANDLER( 0x4B00, JumpHandler )
		//HANDLER( 0x2E00, VelocityHandler )
	};


public:
	PacketHandler( );
	~PacketHandler( );
	void HandleRECV( void * pRawPacket, int size );
	void HandleSEND( void * pRawPacket, int size );
};

extern PacketHandler * pPacketHandler;