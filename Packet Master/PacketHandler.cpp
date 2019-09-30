#include "PacketHandler.hpp"
#include <tuple>
#include "PacketReader.hpp"

void PacketHandler::LoggingHandler( PacketHeader * pPacket, int size )
{
	uintptr_t start = ( uintptr_t ) pPacket->GetPacketData( );
	int real_size = size;
	if ( !start )
	{
		Log( "Can't get packet data" );
		return;
	}
	else
		real_size -= pPacket->GetHeaderSize( );

	for ( int i = 0; i < real_size; i++ )
		printf_s( "%02X", ( ( char* ) start )[ i ] & MAXWORD );

	puts( "" );

	char file_name[ 250 ] = { 0 };
	sprintf_s( file_name, "LOG_ID_%X", ( ( PacketData* ) start )->id & MAXWORD );

	// Full packet
	Utility::DumpPacket( ( char* ) pPacket, size, file_name );
	//Utility::DumpPacket( ( char* ) start, real_size, file_name );
}

#pragma region RECV Handlers
void PacketHandler::PositionHandler( PacketHeader * pPacket, int size )
{
	//Log( "MOVE PKT" );
	PacketReader pReader( pPacket );
	pReader.DiscardBytes( 8 ); // header shit.

	if ( pReader.GetModifier( ) == 3 )
		pReader.DiscardBytes( 10 + 1 ); // 6 + 4 + 1
	else
	{
		pReader.DiscardBytes( 1 );
		auto modifier_2 = pReader.ReadByte( );
		if ( modifier_2 == 0x04 )
			pReader.DiscardBytes( 11 - 2 );
		else if ( modifier_2 == 0x09 )
			pReader.DiscardBytes( 21 - 2 );
		else if ( modifier_2 == 0x7B )
			pReader.DiscardBytes( 33 - 2 );
		else if ( modifier_2 == 0x6B || modifier_2 == 0x39 )
			pReader.DiscardBytes( 29 - 2 );
		else
			pReader.DiscardBytes( 25 - 2 );

		//pReader.DiscardBytes( 25 ); // xdDDD??
	}


	struct Vec3
	{
		float x, y, z;
	};

	if ( !pReader.HasData( ) )
		return;

	Vec3 out_pos = pReader.ReadType<Vec3>( );

	//printf_s( "%.2f %.2f %.2f\n", out_pos.x, out_pos.y, out_pos.z );
	//if ( out_pos.x <= 1.f || out_pos.y <= 1.f || out_pos.z <= 1.f )
	//{
	//	char file_name[ 250 ] = { 0 };
	//	sprintf_s( file_name, "LOG_ID_%X", pPacket->GetPacketData( )->id );
	//	Utility::DumpPacket( ( char* ) pPacket, size, file_name );
	//}
}

void PacketHandler::EntityPosition( PacketHeader * pPacket, int size )
{ 
	PacketReader pReader( pPacket );
	pReader.DiscardBytes( 0x28 ); // not good enough, also happens for all entities, somewhere is probably an ID?
	struct Vec3
	{
		float x, y, z;
	};

	if ( !pReader.HasData( ) )
		return;

	Vec3 out_pos = pReader.ReadType<Vec3>( );

	printf_s( "%.2f %.2f %.2f\n", out_pos.x, out_pos.y, out_pos.z );
}

#pragma endregion


#pragma region SEND Handlers
void PacketHandler::JumpHandler( PacketHeader * pPacket, int size )
{

}

void PacketHandler::VelocityHandler( PacketHeader * pPacket, int size )
{

}

#pragma endregion

PacketHandler::PacketHandler( )
{ 
	this->pIPCHandler = new IPCHandler( );
}

PacketHandler::~PacketHandler( )
{ 
	if( this->pIPCHandler )
		delete this->pIPCHandler;

	this->pIPCHandler = 0;
}

void PacketHandler::HandleRECV( void * pRawPacket, int size )
{

	auto pHeader = ( PacketHeader* ) pRawPacket;
	auto pData = ( PacketData* ) pHeader->GetPacketData( );
	if ( !pData )
	{
		Utility::DumpPacket( ( char* ) pRawPacket, size, "bad_header" );
	}
	else
	{

		auto pHandler = this->pRecvHandlers.find( pData->id );
		if ( pHandler == this->pRecvHandlers.end( ) )
		{
			if ( this->log_recv )
			{
				Log( "RECV ID: 0x%02X | SIZE: %X", pData->id & MAXWORD, size );
				LoggingHandler( pHeader, size );
			}
		}
		else
			( this->*pHandler->second )( pHeader, size ); // witchcraft i know.

		this->pIPCHandler->LogRecv( pHeader, size );
	}
}

void PacketHandler::HandleSEND( void * pRawPacket, int size )
{
	PacketHeader * pHeader = ( PacketHeader* ) pRawPacket;

	auto pHandler = this->pSendHandlers.find( pHeader->GetPacketData( )->id );
	if ( pHandler == this->pSendHandlers.end( ) )
	{
		if ( this->log_send )
			Log( "SEND ID: 0x%02X", pHeader->GetPacketData( )->id & MAXWORD );
	}
	else
		( this->*pHandler->second )( pHeader, size ); // witchcraft i know.

	//if ( pHeader->IsTypeTwo( ) )
	//{
	//	auto pData = pHeader->GetPacketData( );
	//	if ( pData == nullptr )
	//	{
	//		//auto cur_time = GetTime( );
	//		//char uFileName[ 250 ] = { 0 };
	//		//sprintf_s( uFileName, "%s_size-%03X_no_match.bin", cur_time, pEncryptionStruct->unencrypted_size );
	//		//Utility::DumpPacket( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size, uFileName );
	//		// Log unknown packets with flag 0x6... header flag
	//	}
	//	else
	//	{
	//		if ( this->log_ids )
	//			Log( "Type 2 ID: 0x%02X", pHeader->packet_id & 0xFFFF );
	//		if ( pHeader->packet_id == 0x4B00 )
	//		{
	//			// JUMP
	//			//auto cur_time = GetTime( );
	//			//char uFileName[ 250 ] = { 0 };
	//			//sprintf_s( uFileName, "%s_%02X_JUMP.bin", cur_time, pEncryptionStruct->unencrypted_size, pPacketHeader->packet_id );
	//			//Utility::DumpPacket( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size, uFileName );
	//		}
	//		else if ( pHeader->packet_id == 0x2E00 )
	//		{
	//			// VELOCITY
	//			if ( log_bytes )
	//			{
	//				auto start = ( ( uintptr_t ) pData + 20 );
	//				auto difference = start - ( uintptr_t ) pHeader;
	//				auto final_size = size - difference;
	//				for ( int i = 0; i < final_size; i++ )
	//				{
	//					printf_s( "%02X", ( ( char* ) start )[ i ] & 0xFF );
	//					if ( i == 1 || i == 5 || i == 9 || i == 13 || i == 17 || i == 21 )
	//						printf_s( " " );
	//				}
	//				puts( "" );
	//			}
	//			//auto cur_time = GetTime( );
	//			//char uFileName[ 250 ] = { 0 };
	//			//sprintf_s( uFileName, "%s_%02X_POS.bin", cur_time, pEncryptionStruct->unencrypted_size, pPacketHeader->packet_id );
	//			//Utility::DumpPacket( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size, uFileName );
	//		}
	//	}
	//}
	//else
	//{
	//	// fixed size packet
	//	if ( log_ids )
	//		Log( "Type 1 ID: ID %02X", pHeader->packet_id & 0xFFFF );
	//}

}
