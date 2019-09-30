#include "PacketReader.hpp"
#include <dxgi.h>
#include <d3d11.h>

bool PacketReader::InBounds( int size_read )
{

	return ( this->cur_offset + size_read < this->packet_size );
}

PacketReader::PacketReader( PacketHeader * pHeader )
{
	this->pHead = pHeader;
	auto data = pHead->GetPacketData( );
	this->pPacket = ( char* ) data;

	this->packet_id = data->id;
	this->packet_modifier = data->modifier;
	this->packet_size = pHead->data_size;
		//data->payload_size ;
	this->cur_offset = 0;
}

int PacketReader::GetOffset( )
{
	return this->cur_offset;
}

void PacketReader::SetOffset( int new_off )
{
	this->cur_offset = new_off;
}

char PacketReader::GetModifier( )
{
	return this->packet_modifier;
}

bool PacketReader::HasData( )
{
	return InBounds( 1 );
}

unsigned char PacketReader::ReadByte( )
{
	if ( InBounds( 1 ) )
	{
		auto val = *( unsigned char* ) ( pPacket + cur_offset  );
		cur_offset += sizeof( char );
		return val;
	}
	else
	{
		Log( "Out of bounds" );
		return -1;
	}
}

int PacketReader::ReadInt( )
{
	if ( InBounds( sizeof( int ) ) )
	{
		auto val = *( int* ) ( pPacket + cur_offset  );
		cur_offset += sizeof( int );
		return val;
	}
	else
	{
		Log( "Out of bounds" );
		return -1;
	}
}

float PacketReader::ReadFloat( )
{
	if ( InBounds( sizeof( float ) ) )
	{
		auto val = *( float* ) ( pPacket + cur_offset  );
		cur_offset += sizeof( float );
		return val;
	}
	else
	{
		Log( "Out of bounds" );
		return ( float ) -1;
	}
}

short PacketReader::ReadShort( )
{
	if ( InBounds( sizeof( short ) ) )
	{
		auto val = *( short* ) ( pPacket + cur_offset  );
		cur_offset += sizeof( short );
		return val;
	}
	else
	{
		Log( "Out of bounds" );
		return ( short ) -1;
	}
}


void PacketReader::DiscardBytes( int count )
{
	this->cur_offset += count;
}