#pragma once
#include "PacketDefs.hpp"

class PacketReader // Treating these fuckers like streams
{
	const static int hdr_size = 8;

	PacketHeader* pHead;
	int cur_offset;
	short packet_id;
	char packet_modifier; // ?
	char packet_size;
	char * pPacket; // same as Packet Data
	bool InBounds( int size_read );

public:
	PacketReader( PacketHeader* pHeader );
	int GetOffset( );
	void SetOffset( int new_off );

	char GetModifier( );

	bool HasData( );

	unsigned char ReadByte( );
	int ReadInt( );
	float ReadFloat( );
	short ReadShort( );

	// Read a specific type from the stream
	template<typename t>
	t ReadType( )
	{
		if ( InBounds( sizeof( t ) ) )
		{
			auto val = *( t* ) ( pPacket + cur_offset );
			cur_offset += sizeof( t );
			return val;
		}
		else
		{
			Log( "Out of bounds" );
			return t( );
		}
	}

	void DiscardBytes( int count );
};