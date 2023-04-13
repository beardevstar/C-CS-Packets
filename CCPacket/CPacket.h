/*
* ITEM TYPE : string, BYTE[], numerical, bool
* SetItem(field, value)
* 
* GetItemAs...(field)
* --PacketSize--PacketItem[]
*/
#pragma once

#ifndef CPACKET_HEADER
#define CPACKET_HEADER

#include <Windows.h>
#include <string>
#include <map>

using namespace std;

namespace Packet {
	#define ITEM_SIGN 0x62656172	//bear
	#define TYPE_FILED "packet_type"
	#define ID_FILED "client_id"
	#define MAX_PACKET_SIZE (0x800000) //16M
	#define UPLOAD_PACKET_SIZE (0x1000)	//8K
	#define SOCK_BUFSIZE 0x10000	//64K
	
	enum class PacketItemType
	{
		NONE = 0,
		BYTE_ARRAY = 0x3030,
		STRING_ITEM = 0x4040,
		WSTRING_ITEM = 0x8080,
		LONG_ITEM = 0x5050,
		INT_ITEM = 0x7070,
		BOOLEAN_ITEM = 0x6060,
	};
	
	class PacketItem
	{
		typedef struct _PacketItemHeader {
			DWORD signature = ITEM_SIGN;
			DWORD totalLen;
			DWORD nameLen;
			DWORD dataLen;
			PacketItemType type;

		} PACKETITEMHEADER, * LPPACKETITEMHEADER;

		PACKETITEMHEADER header;
		string name;
		LPBYTE data;

		BOOL AddLong(string _name, INT64 _data);
		BOOL AddString(string _name, const char* _data, DWORD len);
	public:
		PacketItem();
		~PacketItem();

		PacketItem(string _name, LPBYTE _data, DWORD _data_len);
		PacketItem(string _name, string _data);
		PacketItem(string _name, wstring _data);
		PacketItem(string _name, INT64 _data);
		PacketItem(string _name, bool _data);
		PacketItem(string _name, int _data);

		PacketItem* Clone();

		INT64 GetAsLong();
		BOOL GetAsBoolean();
		string GetAsString();
		wstring GetAsWString();
		BOOL GetAsByteArray(LPBYTE _buf, DWORD _buflen);

		static PacketItem* FromBytes(LPBYTE _buf);
		BOOL ToBytes(LPBYTE buf, DWORD buf_len);
		BOOL IsValidType() {
			return header.type == PacketItemType::BYTE_ARRAY ||
				header.type == PacketItemType::LONG_ITEM ||
				header.type == PacketItemType::BOOLEAN_ITEM ||
				header.type == PacketItemType::INT_ITEM ||
				header.type == PacketItemType::WSTRING_ITEM ||
				header.type == PacketItemType::STRING_ITEM;
		}
		DWORD GetTotalSize() {
			if (!IsValidType()) return 0;
			return sizeof(PACKETITEMHEADER) + header.nameLen + header.dataLen;
		}
		DWORD GetDataSize() {
			return header.dataLen;
		}
		PacketItemType GetItemType() {
			if (!IsValidType()) return PacketItemType::NONE;
			return header.type;
		}
		string GetItemName() { return name; }
	};

	class CPacket
	{
		map<string, PacketItem*> map;
	public:
		CPacket();
		~CPacket();

		BOOL SetItem(string _name, LPBYTE _data, DWORD _data_len);
		BOOL SetItem(string _name, string _data);
		BOOL SetItem(string _name, const char* _data);
		BOOL SetItem(string _name, const wchar_t* _data);
		BOOL SetItem(string _name, wstring _data);
		BOOL SetItem(string _name, bool _data);
		BOOL SetItem(string _name, INT64 _data);
		BOOL SetItem(string _name, int _data);

		BOOL SetItem(PacketItem* item);

		INT64 GetAsLong(string _name);
		BOOL GetAsBoolean(string _name);
		string GetAsString(string _name);
		wstring GetAsWString(string _name);
		BOOL GetAsByteArray(string _name, LPBYTE _buf, DWORD _buflen);

		DWORD GetItemDataSize(string _name);

		static CPacket* LoadFromByte(LPBYTE _buf, DWORD _len);
		BOOL ToBytes(LPBYTE _buf, DWORD _buflen);
		DWORD GetTotalBytes();
	private:

	};
}

using namespace Packet;
#endif

