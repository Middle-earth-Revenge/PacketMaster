#include "Utility.hpp"

#define PACKET_DUMP_LOC "C:\\Users\\Anderson\\Desktop\\PACKET_DUMPS\\"

char * Utility::GetTime( )
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


void Utility::DumpPacket( char * data, int size, const char * filename )
{
	char uFileName[ 250 ] = { 0 };
	auto cur_time = GetTime( );
	sprintf_s( uFileName, "%s%s-size-%03X-%s.bin", PACKET_DUMP_LOC, cur_time, size, filename );
	std::ofstream out_buffer( uFileName, std::ios::binary );
	out_buffer.write( data, size );
	out_buffer.close( );
}
