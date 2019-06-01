#pragma once
#include <ws2def.h>

// if correct, all this checksum and shit happens in SendAllBuffers, just before the packet is sent
struct PacketHeader
{
	char checksum[ 0x14 ]; // these are actually 0x14, but i fucked up xd
	int id;
	char payload_size; // size of entirety below this header
	char unk;
};

struct PacketHeaderLarge
{
	char checksum[ 0x1A ]; // these are actually 0x14, but i fucked up xd
	int id;
	char payload_size; // size of entirety below this header
	char unk;
};


struct OCCSendTextPacket
{
	static const int id = 0x5C810600;

	union
	{
		struct
		{
			PacketHeader header;
		private:
			char pad[ 0x1E ];
		public:
			int text_length;
			char text[ ];
		} small_checksum ;

		struct
		{
			PacketHeaderLarge header;
		private:
			char pad[ 0x1E ];
		public:
			int text_length;
			char text[ ];
		} big_checksum;
	};
};
/*
	Send buffer comes from

	Argument->258 == WSABUFFERS

*/


class SocketStruct
{
public:
	char pad_0000[ 36 ]; //0x0000
	float something_seconds; //0x0024
	uint32_t TickCount; //0x0028
	char pad_002C[ 132 ]; //0x002C
	class e_scrap *pOutBuffer; //0x00B0
	class BDPS_Array *ppBDPS; //0x00B4
	char pad_00B8[ 4 ]; //0x00B8
	int32_t bdps_count; //0x00BC
	char pad_00C0[ 68 ]; //0x00C0
}; //Size: 0x0104

class BDPS_Array
{
public:
	class BDPS *pBDPS; //0x0000
	char pad_0004[ 52 ]; //0x0004
}; //Size: 0x0038

class PacketChecksumheader
{
public:
	char header_checksum_shit_idk[ 20 ]; //0x0000
	char pad_0014[ 256 ]; //0x0014
}; //Size: 0x0114

class BDPS
{
public:
	class SocketStruct *pParent; //0x0000
	char pad_0004[ 596 ]; //0x0004
	WSABUF packets_start[ 10 ]; //0x0258
	char pad_02A8[ 528 ]; //0x02A8
	class PacketChecksumheader pChecksum; //0x04B8
	char pad_05CC[ 432 ]; //0x05CC
}; //Size: 0x077C

#define concat2(x,y) x##y
#define concat(x,y) concat2(x,y)
#define PAD(x) private: char concat(_pad, __COUNTER__)[x]; public:

struct PacketData
{
	PAD( 8 );
	char * pData;
};

struct PacketEncryptionStuff
{
	int shift_value;
	int encryption_offset;
	PAD( 0xC );
	PacketData * encryptedData;
	PAD( 0x40 );
	int unencrypted_size; // Add 1 to get encrypted size
	PacketData * unencryptedData;
};

#undef PAD
#undef concat
#undef concat2