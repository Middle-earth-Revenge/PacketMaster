#include "PacketDefs.hpp"
#include <map>

const std::map< char, int > header_sizes =
{
	{ 0, 0x14 },
	{ 0xF, 0x14 }, // TODO: CHECK
	{ 3, 0x20 },
	{ 8, 0x1A },
	{ 0xB, 0x26 },// not sure about this one
	{ 0xC, 0x21 },
};

int PacketHeader::GetHeaderSize( )
{
	auto char_val = this->header_flags & 0xF;
	auto entry = header_sizes.find( char_val );
	if ( entry == header_sizes.end( ) )
	{
		return -1;
	}
	else
	{
		return entry->second;
	}
}

PacketData * PacketHeader::GetPacketData( )
{
	char * pPacket = ( char* ) this;
	//return ( PacketData* ) ( pPacket + 0x14 ); // past the header!
// while above is accurate, sometimes header extends past 0x14, still cant figure out what the purpose is though.

	auto is_type_one = [ ]( int flags ) -> bool
	{
		bool out = 0;
		out |= ( flags & HeaderFlags::TYPE_ONE ) == HeaderFlags::TYPE_ONE;
		out |= ( flags & HeaderFlags::TYPE_ONE_TWO ) == HeaderFlags::TYPE_ONE_TWO;
		out |= ( flags & HeaderFlags::TYPE_ONE_THREE ) == HeaderFlags::TYPE_ONE_THREE;
		out |= ( flags & HeaderFlags::TYPE_ONE_FOUR ) == HeaderFlags::TYPE_ONE_FOUR;
		return out;
	};

	if ( ( this->header_flags & HeaderFlags::TYPE_TWO ) == HeaderFlags::TYPE_TWO )
	{
		auto size = GetHeaderSize( );
		if ( size == -1 )
		{
			Log( "No match for header_flag byte 4 %X | %X", this->header_flags & 0xF, this->header_flags );
			return nullptr;
		}

		auto loc = pPacket + size;
		return ( PacketData* ) loc;
	}
	else if ( is_type_one( this->header_flags ) )
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
	return ( ( this->header_flags & HeaderFlags::TYPE_TWO ) == HeaderFlags::TYPE_TWO );
}
