#include "../hook_lib/hook_lib/hooklib.hpp"
#include "PacketDefs.hpp"
#include <ws2def.h>
#include <ctime>
#include <chrono>
#include <iostream>
#include <fstream>
#include <intrin.h>

#define PACKET_DUMP_LOC "C:\\Users\\Anderson\\Desktop\\PACKET_DUMPS\\"

Utility::HookManager * Utility::pHookManager;
#define pHM Utility::pHookManager
FILE * pConsole = 0;

char * GetTime( )
{
	std::tm now;
	std::time_t xd = time( 0 );
	localtime_s( &now, &xd );
	auto cnow = std::chrono::system_clock::now( );
	auto sec = std::chrono::time_point_cast< std::chrono::seconds >( cnow );
	auto fraction = cnow - sec;
	auto ms = std::chrono::duration_cast< std::chrono::milliseconds >( fraction );


	char * buffer = new char[ 50 ];
	memset( buffer, 0, 50 );
	sprintf_s( buffer, 50, "%02d-%02d-%03d", now.tm_min, now.tm_sec, ms.count( ) );
	return buffer;
}

void SendHook( Utility::x86Registers* pRegs )
{
	Log( "SEND" );
}

void RecvHook( Utility::x86Registers* pRegs )
{
	Log( "RECV" );
}

void WSASendHook( Utility::x86Registers* pRegs )
{
	Log( "WSASEND" );
}

void WSARecvHook( Utility::x86Registers* pRegs )
{
	Log( "WSARECV" );
}

//void memcpyHook( Utility::x86Registers* pRegs )
//{ 
//	//auto destStr = (char*)pRegs->edi;
//	//auto srcStr = (char*)pRegs->esi;
//	auto srcStr = ( char* ) pRegs->stack[ 5 ];
//	if ( pRegs->stack[ 5 ] == 0 )
//		return;
//
//	if ( !_stricmp( srcStr, "lovelysun" ) )
//	{
//		auto ret = pRegs->stack[ 3 ];
//		if( ret == (void*)0x00F31BC4 )
//			pRegs->stack[ 5 ] = (void*)"sumochop!";
//		else
//			pRegs->stack[ 5 ] = ( void* )"greatape!";
//
//		Log( "Copying testballs, return address == %p", pRegs->stack[3] );
//	}
//}

void SendPacketHook( Utility::x86Registers* pRegs )
{
//0x00F5AE30
	auto pSocketStruct = ( SocketStruct* ) pRegs->ecx; // double check these stack[ 1 + 4 ] shits
	auto pBDPS = ( BDPS* ) pRegs->stack[ 3 ];

	Log( "Called from %p with sock struct %p, bdps %p", pRegs->stack[ 0 + 4 ], pSocketStruct, pBDPS );
	//auto cur_time = GetTime( );
	//char file_name[ 250 ] = { 0 };
	//auto buffers = pBDPS->packets_start;

	//sprintf_s( file_name, "%s%s_size-%03X_%p.bin", PACKET_DUMP_LOC, cur_time, buffers[ 0 ].len, pSocketStruct );

	//std::ofstream out_buffer( file_name, std::ios::binary );
	//out_buffer.write( buffers[ 0 ].buf, buffers[ 0 ].len );
	//out_buffer.close( );
}

void InitializePacketHook( Utility::x86Registers* pRegs )
{
	struct PacketBitch
	{
		char pad[ 8 ];
		char * pData;
	};

	struct PacketMaster
	{
		int data_offset;
		int data_length;
		PacketBitch* packet_data;
	};


	auto pMaster = ( PacketMaster* ) pRegs->ecx;
	auto dataStart = pMaster->data_offset + pMaster->packet_data->pData;
	if ( !_stricmp( dataStart, "testing" ) )
	{
		//__debugbreak( );
		Log( "Found testing string, return address %p", pRegs->stack[ 2 ] );
	}

}

void EncryptionHook( Utility::x86Registers* pRegs )
{
	//004583B0 -- Encryption Begins here. If we hook here, grab from eax (or esp+8), and subtract 0x6C and cast it to a PACKET_ENCRYPTION_ROUTE, we can access unencrypted and encrypted data with konwn size!
	// better to hook here, we cant get encrypted data at the start! 00458421, using this one, subtract 0xC from eax/ecx

	//auto pEncryptionStruct = ( PacketEncryptionStuff* ) ( pRegs->eax - 0x6C ); // can also find on stack[ 4 ], if you us this one subtract 0x54 instead
	auto pEncryptionStruct = ( PacketEncryptionStuff* ) ( pRegs->eax - 0xC );
	Log( "Encryption struct at %p", pEncryptionStruct );

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
}

void WSASendToHook( Utility::x86Registers* pRegs )
{
	//pRegs->stack[ 2] = first param
	auto buffers = ( LPWSABUF ) pRegs->stack[ 3 ];
	auto buffer_count = ( int ) pRegs->stack[ 4 ];

	//Log( "Sending %d buffers", buffer_count );
	for ( int i = 0; i < buffer_count; i++ )
	{
		//auto packetEnd = buffers[ i ].buf;
		//packetEnd += buffers[ i ].len;
		//packetEnd -= sizeof( "test" );
		OCCSendTextPacket * pPacket = ( OCCSendTextPacket* ) buffers[ i ].buf; // we do not catch things smaller than 4 letters

		if ( buffers[ i ].len >= sizeof( OCCSendTextPacket::small_checksum ) && ( pPacket->small_checksum.header.id == pPacket->id || pPacket->big_checksum.header.id == pPacket->id ) )
		{
			//Log( "Text packet!" );
			//// Going to hunt down packet creation by finding text packets

			//Log( "Buffer %d] %p", i, &buffers[ i ] );
			//auto cur_time = GetTime( );
			//char file_name[ 250 ] = { 0 };
			//sprintf_s( file_name, "%s%s_size-%03X_%d.bin", PACKET_DUMP_LOC, cur_time, buffers[ i ].len, i );

			//std::ofstream out_buffer( file_name, std::ios::binary );
			//out_buffer.write( buffers[ i ].buf, buffers[ i ].len );
			//out_buffer.close( );
		}
	}

	//Log( "Send complete" );
	//Log( "WSASENDTO" );
}

void WSARecvFromHook( Utility::x86Registers* pRegs )
{
	//Log( "WSARECVFROM" );
}

//std::ofstream outLog;
//
//void printfHook( Utility::x86Registers* pRegs )
//{
//	if ( outLog.is_open( ) == false )
//		outLog.open( PACKET_DUMP_LOC"\\dumped_log.log", std::ios::trunc );
//
//	//00F29110
//	auto fmt = ( char* ) pRegs->ecx;
//
//	char buffer[ 500 ] = { 0 };
//	sprintf_s( buffer, "%s from address %p", fmt, pRegs->stack[ 0 ] );
//	Log( "%s", buffer );
//	outLog << buffer << "\n\n";
//	outLog.flush( );
//}
//
//void LogHook( Utility::x86Registers* pRegs /*char * format*/ )
//{
//	//if ( outLog.is_open( ) == false )
//		//outLog.open( PACKET_DUMP_LOC"\\dumped_log.log", std::ios::trunc );
//
//	auto format = ( char* ) pRegs->stack[ 3 ];
//	auto retAddr = pRegs->stack[ 2 ];
//
//	//char buffer[ 500 ] = { 0 };
//	//sprintf_s( buffer, "Logged %s from address %p", format, retAddr );
//	//Log( "%s", buffer );
//	//outLog << buffer << "\n";
//	//outLog.flush( );
//}


unsigned long __stdcall MainShit( PVOID ctx )
{
	HMODULE hThis = ( HMODULE ) ctx;

	CreateDirectory( "C:\\Users\\Anderson\\Desktop\\PACKET_DUMPS", 0 );

	Log( "Hooking Send & Recv functions daddy" );

	auto ws32 = GetModuleHandleA( "WS2_32.dll" );
	Log( "ws2_32 at %p", ws32 );

	auto send = GetProcAddress( ws32, "send" );
	Log( "send at %p", send );
	auto recv = GetProcAddress( ws32, "recv" );
	Log( "recv at %p", recv );

	auto WSASend = GetProcAddress( ws32, "WSASend" );
	Log( "WSASend at %p", WSASend );
	auto WSARecv = GetProcAddress( ws32, "WSARecv" );
	Log( "WSARecv at %p", WSARecv );

	auto WSASendTo = GetProcAddress( ws32, "WSASendTo" );
	Log( "WSASendTo at %p", WSASendTo );
	auto WSARecvFrom = GetProcAddress( ws32, "WSARecvFrom" );
	Log( "WSARecvFrom at %p", WSARecvFrom );

	pHM->HookFunctionExt( ( uintptr_t ) send, ( uintptr_t ) SendHook );
	pHM->HookFunctionExt( ( uintptr_t ) recv, ( uintptr_t ) RecvHook );

	pHM->HookFunctionExt( ( uintptr_t ) WSASend, ( uintptr_t ) WSASendHook );
	pHM->HookFunctionExt( ( uintptr_t ) WSARecv, ( uintptr_t ) WSARecvHook );

	pHM->HookFunctionExt( ( uintptr_t ) WSASendTo, ( uintptr_t ) WSASendToHook );
	pHM->HookFunctionExt( ( uintptr_t ) WSARecvFrom, ( uintptr_t ) WSARecvFromHook );

	//0x00F31B40
	//pHM->HookFunctionExt( ( uintptr_t ) 0x00F31B40, ( uintptr_t ) InitializePacketHook, 6 );
	pHM->HookFunctionExt( ( uintptr_t ) 0x00F5AE30, ( uintptr_t ) SendPacketHook, 8 );
	//004583B0 -- Encryption Begins here. If we hook here, grab from eax (or esp+8), and subtract 0x54 and cast it to a PACKET_ENCRYPTION_ROUTE, we can access unencrypted and encrypted data with konwn size!
	// better here: 00458421
	pHM->HookFunctionExt( ( uintptr_t ) 0x00458421, ( uintptr_t ) EncryptionHook, 7 );


	//pHM->HookFunctionExt( 0x00F29110, ( uintptr_t ) printfHook );
	//pHM->HookFunctionExt( 0x00402FA0, ( uintptr_t ) LogHook, 6 );

	while ( true )
	{
		if ( GetAsyncKeyState( VK_END ) & 1 )
		{
			//pHM->UnhookFunction( ( uintptr_t ) send );
			//pHM->UnhookFunction( ( uintptr_t ) recv );
			//pHM->UnhookFunction( ( uintptr_t ) WSASend );
			//pHM->UnhookFunction( ( uintptr_t ) WSARecv );
			//pHM->UnhookFunction( ( uintptr_t ) WSASendTo );
			//pHM->UnhookFunction( ( uintptr_t ) WSARecvFrom );
			//pHM->UnhookFunction( ( uintptr_t ) 0x61EC5090 );
			//pHM->UnhookFunction( ( uintptr_t ) 0x00F5AE30 );
			//pHM->UnhookFunction( ( uintptr_t ) 0x00F29110 );
			delete Utility::pHookManager;


			//outLog.close( );
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