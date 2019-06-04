#pragma once
#include "standard_includes.hpp"

namespace Utility {

	char * GetTime( );

	// Writes packet to PACKET_DUMP_LOC\\TIME-size-<size>-file_name.bin. Packet_Dump_Loc found in Utility.cpp
	void DumpPacket( char * data, int size, const char * filename );
}