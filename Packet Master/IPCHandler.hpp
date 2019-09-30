#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class IPCHandler
{
	HANDLE hPipe;
public:
	IPCHandler( );
	~IPCHandler( );
	void LogSend( void * sendData, int size );
	void LogRecv( void * recvData, int size );
};