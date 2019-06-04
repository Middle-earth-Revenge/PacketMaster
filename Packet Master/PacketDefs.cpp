#include "PacketDefs.hpp"
#include <map>

const std::map< char, int > header_sizes =
{
	{ 0, 0x14 },
	{ 3, 0x20 },
	{ 8, 0x1A },
	{ 0xB, 0x26 },// not sure about this one
	{ 0xC, 0x21 },
};

PacketData * PacketHeader::GetPacketData( )
{
	char * pPacket = ( char* ) this;

	if ( ( this->header_flags & HeaderFlags::TYPE_TWO ) == HeaderFlags::TYPE_TWO )
	{
		auto GetHeaderSize = [ pPacket, this ]( const std::map< char, int > pMap )
		{
			auto char_val = this->header_flags & 0xF;
			auto entry = pMap.find( char_val );
			if ( entry == pMap.end( ) )
			{
				return -1;
			}
			else
			{
				return entry->second;
			}
		};

		auto size = GetHeaderSize( header_sizes );
		if ( size == -1 )
		{
			Log( "No match for header_flag byte 4 %X | %X", this->header_flags & 0xF, this->header_flags );
			return nullptr;
		}

		auto loc = pPacket + size;
		return ( PacketData* ) loc;
	}
	else if ( ( this->header_flags & HeaderFlags::TYPE_ONE ) == HeaderFlags::TYPE_ONE || ( this->header_flags & HeaderFlags::TYPE_ONE_TWO ) == HeaderFlags::TYPE_ONE_TWO )
	{
		// perhaps walk until we hit 0x3E912300, actually, also found 0x3E91A100, perhaps these are packet IDs?
		// these were all found at location 0x10
		return ( PacketData* ) ( pPacket + 0x10 );
	}
	else
	{
		Log( "What the fuck? How sway?" );
	}


	return nullptr;
}

bool PacketHeader::IsTypeTwo( )
{
	return ( (this->header_flags & HeaderFlags::TYPE_TWO) == HeaderFlags::TYPE_TWO );
}
