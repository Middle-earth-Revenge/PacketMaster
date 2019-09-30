#include "IPCHandler.hpp"
#include "standard_includes.hpp"

IPCHandler::IPCHandler( )
{
	this->hPipe = CreateFileA( R"(\\.\pipe\Gatekeeper)", GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
	if ( !this->hPipe )
	{
		Log( "Pipe not open %d", GetLastError( ) );
	}
}

IPCHandler::~IPCHandler( )
{
	CloseHandle( hPipe );
}

void IPCHandler::LogSend( void * sendData, int size )
{
		//PRT|S/R|TIME|SIZE|DATA for now.
	auto time = GetTickCount( );

	char *writeBuffer = new char[ size + 0x200 ]; // nice stack u got there.
	memset( writeBuffer, 0, size + 0x200 );
	sprintf_s( writeBuffer, size + 0x200, "%d|%u|%d|%d|", 0, 0, time, size ); // copy bytes to the endddddddddd
	memcpy( writeBuffer + strlen( writeBuffer ), sendData, size ); // good?

	WriteFile( hPipe, writeBuffer, size + 0x200, 0, 0 );
	delete writeBuffer;
}

void IPCHandler::LogRecv( void * recvData, int size )
{
		//PRT|S/R|TIME|SIZE|DATA for now.
	auto time = GetTickCount( );

	char *writeBuffer = new char[ size + 0x200 ]; // nice stack u got there.
	memset( writeBuffer, 0, size + 0x200 );
	sprintf_s( writeBuffer, size + 0x200, "%d|%u|%d|%d|", 0, 1, time, size ); // copy bytes to the endddddddddd
	memcpy( writeBuffer + strlen( writeBuffer ), recvData, size ); // good?

	WriteFile( hPipe, writeBuffer, size + 0x200, 0, 0 );
	delete writeBuffer;
}
