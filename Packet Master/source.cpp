#include "PacketDefs.hpp"
#include "Utility.hpp"

Utility::HookManager * Utility::pHookManager;
#define pHM Utility::pHookManager
FILE * pConsole = 0;

const static int log_ids = 0;
const static int log_bytes = 0;

void DecryptionHook( Utility::x86Registers* pRegs ) // not realya  decrypt hook but whatever.
{
	auto pSocketStruct = ( SocketStruct* ) pRegs->ebp;
	auto size = pRegs->edi;
	if ( size > 0 )
	{
		auto pPacket = pSocketStruct->pReceived->pUnencryptedData; // this is not quite done! TODO: Find places that access the read-encrypted data
		if ( pPacket )
		{
			//Log( "%X", size );
			//auto cur_time = GetTime( );
			//char uFileName[ 250 ] = { 0 };
			//sprintf_s( uFileName, "%s_size-%03X_RECV", cur_time, size );
			//Utility::DumpPacket( pPacket->pData, size, uFileName );

			auto pHeader = ( PacketHeader* ) pPacket->pData;
			auto pData = pHeader->GetPacketData( );
			if ( !pData )
			{
				Utility::DumpPacket( pPacket->pData, size, "bad_header" );
			}

			if ( log_ids )
				Log( "RECV ID: 0x%02X", pHeader->packet_id & 0xFFFF );

			if ( pHeader->packet_id == 0x3900 ) // these sizes vary, but i can make out position in quite a few of them. Need to get their data, DUH
			{
				//Log( "MOVE PKT" );
				if ( log_bytes )
				{
					auto start = ( uintptr_t ) pData;
					auto difference = start - ( uintptr_t ) pPacket->pData;
					auto final_size = size - difference;
					for ( int i = 0; i < final_size; i++ )
					{
						if ( i < 19 )
							continue;
						printf_s( "%02X", ( ( char* ) start )[ i ] & 0xFF );
						if ( i == 2 || i == 18 || i == 22 || i == 26 || i == 30 )
							printf_s( " %d ", i );

					}
					puts( "" );
				}

#pragma pack(push, 1)
				struct MoveTest
				{
					PAD( 0x13 );
					float x;
					float y;
					float z;
				};
#pragma pack(pop)

				MoveTest * pTest = ( MoveTest* ) pData;
				printf_s( "%.2f %.2f %.2f\n", pTest->x, pTest->y, pTest->z );
				// POS PACKET
			}

		}
		else
		{
			Log( "NO PACKET, HOW?" );
		}
	}
	else
	{
		Log( "Size zero??" );
	}
}

void EncryptionHook( Utility::x86Registers* pRegs )
{
	//	004583B0 -- Encryption Begins here. If we hook here, grab from eax (or esp+8), and subtract 0x6C and cast it to a PACKET_ENCRYPTION_ROUTE, we can access unencrypted and encrypted data with konwn size!
	// better to hook here, we cant get encrypted data at the start! 00458421, using this one, subtract 0xC from eax/ecx
	// Changed my mind, go hook from start (004583B0), get SocketStruct from EBP - 0xA8, we can then get sent and received packets unencrypted i think??

	//auto pEncryptionStruct = ( PacketEncryptionStuff* ) ( pRegs->eax - 0x6C ); // can also find on stack[ 4 ], if you us this one subtract 0x54 instead
	auto pEncryptionStruct = ( PacketEncryptionStuff* ) ( pRegs->eax - 0xC );
	auto pSocketStruct = ( SocketStruct* ) ( pRegs->ebp - 0xA8 );
	auto pDecryptedSend = pSocketStruct->pDecryptedSendPacket->pData;
	//Log( "Encryption struct at %p", pEncryptionStruct );

	PacketHeader * pSendPacketHeader = ( PacketHeader* ) pDecryptedSend;

	if ( pSendPacketHeader->IsTypeTwo( ) )
	{
		auto pData = pSendPacketHeader->GetPacketData( );
		if ( pData == nullptr )
		{
			//auto cur_time = GetTime( );
			//char uFileName[ 250 ] = { 0 };
			//sprintf_s( uFileName, "%s_size-%03X_no_match.bin", cur_time, pEncryptionStruct->unencrypted_size );
			//Utility::DumpPacket( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size, uFileName );
			// Log unknown packets with flag 0x6... header flag
		}
		else
		{
			if ( log_ids )
				Log( "Type 2 ID: 0x%02X", pSendPacketHeader->packet_id & 0xFFFF );

			if ( pSendPacketHeader->packet_id == 0x4B00 )
			{
				// JUMP
				//auto cur_time = GetTime( );
				//char uFileName[ 250 ] = { 0 };
				//sprintf_s( uFileName, "%s_%02X_JUMP.bin", cur_time, pEncryptionStruct->unencrypted_size, pPacketHeader->packet_id );
				//Utility::DumpPacket( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size, uFileName );
			}
			else if ( pSendPacketHeader->packet_id == 0x2E00 )
			{
				// VELOCITY

				if ( log_bytes )
				{
					auto start = ( ( uintptr_t ) pData + 20 );
					auto difference = start - ( uintptr_t ) pSendPacketHeader;
					auto final_size = pEncryptionStruct->unencrypted_size - difference;
					for ( int i = 0; i < final_size; i++ )
					{
						printf_s( "%02X", ( ( char* ) start )[ i ] & 0xFF );
						if ( i == 1 || i == 5 || i == 9 || i == 13 || i == 17 || i == 21 )
							printf_s( " " );

					}
					puts( "" );
				}

				//auto cur_time = GetTime( );
				//char uFileName[ 250 ] = { 0 };
				//sprintf_s( uFileName, "%s_%02X_POS.bin", cur_time, pEncryptionStruct->unencrypted_size, pPacketHeader->packet_id );
				//Utility::DumpPacket( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size, uFileName );
			}
		}
	}
	else
	{
		// fixed size packet
		if ( log_ids )
			Log( "Type 1 ID: ID %02X", pSendPacketHeader->packet_id & 0xFFFF );
	}

	/*
		auto cur_time = GetTime( );
		char uFileName[ 250 ] = { 0 };
		sprintf_s( uFileName, "%s%s_size-%03X_unencrypted.bin", PACKET_DUMP_LOC, cur_time, pEncryptionStruct->unencrypted_size );
		std::ofstream out_buffer( uFileName, std::ios::binary );
		out_buffer.write( pEncryptionStruct->unencryptedData->pData, pEncryptionStruct->unencrypted_size );
		out_buffer.close( );
		sprintf_s( uFileName, "%s%s_size-%03X_encrypted.bin", PACKET_DUMP_LOC, cur_time, pEncryptionStruct->unencrypted_size );
		out_buffer.open( uFileName, std::ios::binary );
		out_buffer.write( pEncryptionStruct->encryptedData->pData, pEncryptionStruct->unencrypted_size + 1 );
		out_buffer.close( );
	*/
}

unsigned long __stdcall MainShit( PVOID ctx )
{
	HMODULE hThis = ( HMODULE ) ctx;

	CreateDirectory( "C:\\Users\\Anderson\\Desktop\\PACKET_DUMPS", 0 );

	Log( "Hooking Send & Recv functions daddy" );

	//auto ws2_32 = GetModuleHandleA( "WS2_32.dll" );
	//auto ws32 = GetModuleHandleA( "WSOCK32.dll" );
	//auto send = GetProcAddress( ws2_32, "send" );
	//auto recv = GetProcAddress( ws2_32, "recv" );
	//auto recvfrom = GetProcAddress( ws32, "recvfrom" );
	//auto WSASend = GetProcAddress( ws2_32, "WSASend" );
	//auto WSARecv = GetProcAddress( ws2_32, "WSARecv" );
	//auto WSASendTo = GetProcAddress( ws2_32, "WSASendTo" );
	//auto WSARecvFrom = GetProcAddress( ws2_32, "WSARecvFrom" );


	//pHM->HookFunctionExt( ( uintptr_t ) send, ( uintptr_t ) SendHook );
	//pHM->HookFunctionExt( ( uintptr_t ) recv, ( uintptr_t ) RecvHook );
	//pHM->HookFunctionExt( ( uintptr_t ) recvfrom, ( uintptr_t ) RecvFromHook );
	//pHM->HookFunctionExt( ( uintptr_t ) WSASend, ( uintptr_t ) WSASendHook );
	//pHM->HookFunctionExt( ( uintptr_t ) WSARecv, ( uintptr_t ) WSARecvHook );
	//pHM->HookFunctionExt( ( uintptr_t ) WSASendTo, ( uintptr_t ) WSASendToHook );
	//pHM->HookFunctionExt( ( uintptr_t ) WSARecvFrom, ( uintptr_t ) WSARecvFromHook );
	//TODO: Sig scan these.
	// 53 55 56 57 8b 7c 24 14 8b d9 8b 8f b0 04 0 0
	//pHM->HookFunctionExt( ( uintptr_t ) 0x00F6F920, ( uintptr_t ) SendPacketHook, 8 );
	
	
	//004583B0 -- Encryption Begins here. If we hook here, grab from eax (or esp+8), and subtract 0x54 and cast it to a PACKET_ENCRYPTION_ROUTE, we can access unencrypted and encrypted data with konwn size!
	// better here: 00458421
	// 8b 74 24 48 89 4c 24 0c 57 - 0x1B
	pHM->HookFunctionExt( ( uintptr_t ) 0x004581F0, ( uintptr_t ) EncryptionHook, 7 );
	// hooking end of recv function, think theyre decrypted by then? Hope so! // ff d0 83 7c 24 18 00 8d 4c
	pHM->HookFunctionExt( ( uintptr_t ) 0x00F6CC81, ( uintptr_t ) DecryptionHook, 5, false );

	while ( true )
	{
		if ( GetAsyncKeyState( VK_END ) & 1 )
		{
			delete Utility::pHookManager;

			Log( "Console closed." );
			fclose( pConsole );
			FreeConsole( );
			FreeLibraryAndExitThread( hThis, 0 );
		}
		Sleep( 1 );
	}

	return 0;
}

BOOL DllMain( HMODULE hThis, DWORD dw, LPVOID )
{
	if ( dw == DLL_PROCESS_ATTACH )
	{
		AllocConsole( );
		freopen_s( &pConsole, "CONOUT$", "w", stdout );

		Utility::pHookManager = new Utility::HookManager( );
		CreateThread( 0, 0, MainShit, hThis, 0, 0 );
	}

	return TRUE;
}