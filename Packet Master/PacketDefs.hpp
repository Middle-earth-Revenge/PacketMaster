#pragma once
#include "standard_includes.hpp"

#define concat2(x,y) x##y
#define concat(x,y) concat2(x,y)
#define PAD(x) private: char concat(_pad, __COUNTER__)[x]; public:

#define NO_PADDING() __pragma(pack(push, 1))
#define RESTORE_PADDING() __pragma(pack(pop))


enum HeaderFlags : int
{
	TYPE_ONE_TWO = 0x0400000, // xd
	TYPE_ONE_THREE = 0x800000, // size 0x20
	TYPE_ONE = 0x2000000,	// encompasses all 0x2... header flags, known packet sizes
	TYPE_ONE_FOUR = 0x4000000, // 0x40 size
	/*
		0x2000008	// full size 1A
		0x200000B	// full size 26
		0x2400008 	// full size 1E
		0x240000B	// full size 2A
		0x240000F	// full Size 32
	*/
	TYPE_TWO = 0x6000000,	// encompasses all 0x6... header flags, unknown size packets, known header sizes 
	//Apparently the 6 means its a data packet.

	/*
		// 0x6000003 = 0x20 size header
		// 0x6000000 = 0x14 size header
		// 0x6000008 = 0x1A size header
		// 0x6400008 = 0x1E size header
		// 0x600000B = 0x26 size header
	*/

	// all these comments about known/unknown packet sizes are incorrect.
};

NO_PADDING()
struct PacketData
{
	//char id;
	//short unk;
	short id;
	char modifier; // ?
	short sequence_id; // 0->whatever
	short unk_2;
	char payload_size; // 0x7 // maybe short. Size of actual data for given packet-type
	// from here is packet-specific data, i.e. text shit
};
RESTORE_PADDING()

// Usable for Fixed packets, use GetPacketData for non-fixed packets.
struct PacketHeader
{
	int GetHeaderSize( );
	PacketData * GetPacketData( );
	bool IsTypeTwo( );

	short session_id;	// 0 no clue, but doesnt change encrypted vs unencrypted.
	PAD( 1 );
	char data_size;	// contains size of packet sans header! (0x14 bytes always)
	HeaderFlags header_flags;	// 4 Bitfield probably, different values = different size headers and/or different sized packets
	// Parses header_flags to find the start of the raw packet data (can vary immensely)
	// Probably not related to size! This is most likely how the packet is meant to be read!!!
	/* i think header flags is also used to say the exact packet they are if its a fixed packet,
	why would a fixed-size packet tell you what its size is? No reason unless its predetermined,
	and if its predetermined, why not use that predetermination to tell you what kind of packet it is?*/
				// 0 + 0x8 == sequence_id (int or short at 0xA)
	PAD( 8 ); // 0 + 0xC == checksum
	short tick_maybe;
	short _id;
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