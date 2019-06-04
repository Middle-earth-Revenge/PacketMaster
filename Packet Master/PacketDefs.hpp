#pragma once
#include "standard_includes.hpp"

#define concat2(x,y) x##y
#define concat(x,y) concat2(x,y)
#define PAD(x) private: char concat(_pad, __COUNTER__)[x]; public:
enum HeaderFlags : int
{
	TYPE_ONE_TWO = 0x0400000, // xd
	TYPE_ONE = 0x2000000,	// encompasses all 0x2... header flags, known packet sizes
	/*
		0x2000008	// full size 1A
		0x200000B	// full size 26
		0x2400008 	// full size 1E
		0x240000B	// full size 2A
		0x240000F	// full Size 32
	*/
	TYPE_TWO = 0x6000000,	// encompasses all 0x6... header flags, unknown size packets, known header sizes
	/*
		// 0x6000003 = 0x20 size header
		// 0x6000000 = 0x14 size header
		// 0x6000008 = 0x1A size header
		// 0x6400008 = 0x1E size header
		// 0x600000B = 0x26 size header
	*/

	// all these comments about known/unknown packet sizes are incorrect.
};

struct PacketData
{
	int packet_start; // varies from shit like 0x10002 to 0x14002, those are the 2 seen so far, found in packets with undetermined sizes
	PAD( 0x9 );
	char data_size;
	char unk;
	PAD( 8 );
	short tick_maybe;
	short id;
	// from here is packet-specific data, i.e. text shit
};


// Usable for Fixed packets, use GetPacketData for non-fixed packets.
struct PacketHeader
{
	PacketData * GetPacketData( );
	bool IsTypeTwo( );

	short session_id;	// 0 no clue, but doesnt change encrypted vs unencrypted.
	// cant be. 3 recv packets from myself have 3 different values.
	short packet_id;	// 2 contains some information, if bit 0 is 0, it's a fixed size packet, otherwise size is unknown (i.e. text packet)
	HeaderFlags header_flags;	// 4 Bitfield probably, different values = different size headers and/or different sized packets
	// Parses header_flags to find the start of the raw packet data (can vary immensely)
	/* i think header flags is also used to say the exact packet they are if its a fixed packet,
	why would a fixed-size packet tell you what its size is? No reason unless its predetermined,
	and if its predetermined, why not use that predetermination to tell you what kind of packet it is?*/
				// 0 + 0x8 == sequence_id (int or short at 0xA)
	PAD( 8 ); // 0 + 0xC == checksum
	short tick_maybe;
	short id;
};

struct PrivateMessagePacket : PacketData
{
	static const int _id = 0xC7100000;
	//PAD( 8 );
	//int id;
	PAD( 2 );
	char character_count; // is a wchar so * 2 for full shit
	wchar_t message[ 1 ];
	PAD( 11 );
	int id2; // same ID as above.
	PAD( 2 );
	char recipient_size;
	wchar_t recipient[ ];
};

// OBSOLETE
struct OCCSendTextPacket
{
	static const int id = 0x5C810600;

	PacketHeader header;
private:
	char pad[ 0x1E ];
public:
	int text_length;
	char text[ ];
};
/*
	Send buffer comes from

	Argument->258 == WSABUFFERS

*/

struct DataPointer
{
	PAD( 8 );
	char * pData;
};

struct ReceivedPacket
{
	PAD( 0x120 );
	DataPointer * pEncryptedData;
	PAD( 24 );
	DataPointer * pUnencryptedData;
};

struct SocketStruct
{
	PAD( 36 ); //0x0000
	float something_seconds; //0x0024
	uint32_t TickCount; //0x0028
	PAD(132) //0x002C
	DataPointer *pDecryptedSendPacket; //0x00B0
	class BDPS_Array *ppBDPS; //0x00B4
	char pad_00B8[ 4 ]; //0x00B8
	int32_t bdps_count; //0x00BC
	ReceivedPacket * pReceived;
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



struct PacketEncryptionStuff
{
	int shift_value;
	int encryption_offset;
	PAD( 0xC );
	DataPointer * encryptedData;
	PAD( 0x40 );
	int unencrypted_size; // Add 1 to get encrypted size
	DataPointer * unencryptedData;
};

//#undef PAD
//#undef concat
//#undef concat2