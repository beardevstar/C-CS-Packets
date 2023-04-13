#include "pch.h"
#include "CPacket.h"

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

CPacket::CPacket()
{

}

CPacket::~CPacket()
{
	std::map<std::string, PacketItem*>::iterator it = map.begin();
	// Iterate through the map and print the elements
	while (it != map.end())
	{
		PacketItem* item = it->second;
		delete item;
		++it;
	}
}

BOOL CPacket::SetItem(std::string _name, LPBYTE _data, DWORD _data_len)
{
	map[_name] = new PacketItem(_name, _data,_data_len);
	return 0;
}

BOOL CPacket::SetItem(std::string _name, std::string _data)
{
	map[_name] = new PacketItem(_name, _data);
	return 0;
}

BOOL Packet::CPacket::SetItem(string _name, const char* _data)
{
	map[_name] = new PacketItem(_name, string(_data));
	return 0;
}

BOOL Packet::CPacket::SetItem(string _name, const wchar_t* _data)
{
	map[_name] = new PacketItem(_name, wstring(_data));
	return 0;
}

BOOL Packet::CPacket::SetItem(string _name, wstring _data)
{
	map[_name] = new PacketItem(_name, _data);
	return 0;
}

BOOL CPacket::SetItem(std::string _name, bool _data)
{
	map[_name] = new PacketItem(_name, _data);
	return 0;
}

BOOL CPacket::SetItem(std::string _name, INT64 _data)
{
	map[_name] = new PacketItem(_name, _data);
	return TRUE;
}

BOOL CPacket::SetItem(std::string _name, int _data)
{
	map[_name] = new PacketItem(_name, _data);
	return TRUE;
}

BOOL CPacket::SetItem(PacketItem* item)
{
	if (!item || item->GetItemType() == PacketItemType::NONE) return FALSE;
	
	string name = item->GetItemName();
	PacketItem* newItem = item->Clone();
	if (!newItem) return FALSE;

	map[name] = newItem;
	return TRUE;
}

INT64 CPacket::GetAsLong(std::string _name)
{
	PacketItem* item = map[_name];
	if (!item) return 0;

	return item->GetAsLong();
}

BOOL CPacket::GetAsBoolean(std::string _name)
{
	PacketItem* item = map[_name];
	if (!item) return FALSE;

	return item->GetAsBoolean();
}

std::string CPacket::GetAsString(std::string _name)
{
	PacketItem* item = map[_name];
	if (!item) return string();

	return item->GetAsString();
}

wstring Packet::CPacket::GetAsWString(string _name)
{
	PacketItem* item = map[_name];
	if (!item) return wstring();

	return item->GetAsWString();
}

BOOL CPacket::GetAsByteArray(std::string _name, LPBYTE _buf, DWORD _buflen)
{
	PacketItem* item = map[_name];
	if (!item) return FALSE;

	return item->GetAsByteArray(_buf, _buflen);
}

DWORD CPacket::GetItemDataSize(std::string _name)
{
	PacketItem* item = map[_name];
	if (!item) return 0;
	return item->GetDataSize();
}

CPacket* CPacket::LoadFromByte(LPBYTE _buf, DWORD _len)
{
	CPacket* packet = new CPacket();

	if (!_buf) return NULL;
	DWORD totalBytes = *((DWORD*)_buf), copied = sizeof(DWORD);
	if (_len < totalBytes) return NULL;
	
	while (totalBytes > copied) {
		PacketItem* item = PacketItem::FromBytes(_buf + copied);
		if (!item) {
			delete packet;
			return NULL;
		}
		DWORD itemSize = item->GetTotalSize();
		packet->SetItem(item);
		delete item;
		
		if (itemSize <= 0 || itemSize > (totalBytes - copied)) {
			delete packet;
			return NULL;
		}
		copied += itemSize;
	}
	return packet;
}
//packet bytes + itemlist = 
BOOL CPacket::ToBytes(LPBYTE _buf, DWORD _buflen)
{
	DWORD totalBytes = GetTotalBytes();
	if (map.empty() || !_buf || totalBytes > _buflen) return FALSE;

	//set totalBytes
	memcpy(_buf, &totalBytes, sizeof(DWORD));

	DWORD copied = sizeof(DWORD);
	std::map<std::string, PacketItem*>::iterator it = map.begin();
	// Iterate through the map and print the elements
	while (it != map.end())
	{
		PacketItem* item = it->second;
		DWORD itemSize = item->GetTotalSize();
		LPBYTE temp = (LPBYTE)GlobalAlloc(GPTR, itemSize);
		if (!temp) return FALSE;

		if (!item->ToBytes(temp, itemSize)) return FALSE;

		memcpy(_buf + copied, temp, itemSize);
		copied += itemSize;
		++it;
		if(temp) GlobalFree(temp);
	}
	return TRUE;
}

DWORD CPacket::GetTotalBytes()
{
	if (map.empty()) return 0;
	DWORD totalSize = sizeof(DWORD);
	// Get an iterator pointing to the first element in the map
	std::map<std::string, PacketItem*>::iterator it = map.begin();
	// Iterate through the map and print the elements
	while (it != map.end())
	{
		PacketItem* item = it->second;
		int size = item->GetTotalSize();
		totalSize += item->GetTotalSize();
		++it;
	}
	return totalSize;
}

BOOL PacketItem::AddLong(std::string _name, INT64 _data)
{
	
	header.type = PacketItemType::LONG_ITEM;
	name = _name;
	header.nameLen = (int)name.length();
	header.dataLen = sizeof(INT64);

	data = (LPBYTE)GlobalAlloc(GPTR, sizeof(INT64));
	if (!data) return FALSE;

	memcpy(data, &_data, sizeof(INT64));
	return TRUE;
}

BOOL PacketItem::AddString(std::string _name, const char* _data, DWORD _len)
{
	if (_data == NULL) return FALSE;

	memset(&header, 0, sizeof(PACKETITEMHEADER));

	header.type = PacketItemType::STRING_ITEM;
	name = _name;
	header.nameLen = (int)name.length();
	header.dataLen = _len;

	data = (LPBYTE)GlobalAlloc(GPTR, _len);
	if (!data) return FALSE;

	memcpy(data, _data, _len);
	return TRUE;
}


PacketItem::PacketItem()
{
	name = std::string();
	data = NULL;
}

PacketItem::PacketItem(std::string _name, LPBYTE _data, DWORD _data_len)
{
	if (_data == NULL) return;
	memset(&header, 0, sizeof(PACKETITEMHEADER));
	
	header.type = PacketItemType::BYTE_ARRAY;
	name = _name;
	header.nameLen = (int)name.length();
	header.dataLen = _data_len;

	data = (LPBYTE)GlobalAlloc(GPTR, _data_len);
	if (!data) return;

	memcpy(data, _data, _data_len);
	//totalLen will be set whenn call toBytes
}

PacketItem::PacketItem(std::string _name, std::string _data)
{
	wstring real = utf8_decode(_data);
	AddString(_name, (const char*)real.c_str(), (int)real.length() * sizeof(wchar_t));
}

Packet::PacketItem::PacketItem(string _name, wstring _data)
{
	AddString(_name, (const char*)_data.c_str(), (int)_data.length() * sizeof(wchar_t));
	header.type = PacketItemType::WSTRING_ITEM;
}

PacketItem::PacketItem(std::string _name, INT64 _data)
{
	AddLong(_name, _data);
}

PacketItem::PacketItem(std::string _name, bool _data)
{
	AddLong(_name, INT64(_data));
	header.type = PacketItemType::BOOLEAN_ITEM;
}

PacketItem::PacketItem(std::string _name, int _data)
{
	AddLong(_name, INT64(_data));
	header.type = PacketItemType::INT_ITEM;
}

PacketItem* PacketItem::Clone()
{
	PacketItem* item = new PacketItem();
	if (header.dataLen <= 0) return NULL;
	
	item->data = (LPBYTE)GlobalAlloc(GPTR, header.dataLen);
	if (!item->data || !data) return NULL;

	item->name = name;
	memcpy(item->data, data, header.dataLen);
	memcpy(&item->header, &header, sizeof(PACKETITEMHEADER));
	return item;
}

INT64 PacketItem::GetAsLong()
{
	if (!data || header.dataLen < sizeof(INT64))
		return 0;
	if(header.type != PacketItemType::LONG_ITEM || header.type != PacketItemType::INT_ITEM)
		return *((LPLONG)data);
	return 0;
}

BOOL PacketItem::GetAsBoolean()
{
	if (!data || header.dataLen < sizeof(INT64) || header.type != PacketItemType::BOOLEAN_ITEM)
		return FALSE;
	return *((INT64*)data) == 1;
}

std::string PacketItem::GetAsString()
{
	wstring str = GetAsWString();
	return utf8_encode(str);
}

wstring Packet::PacketItem::GetAsWString()
{
	if (!data) return std::wstring();
	if (header.type != PacketItemType::STRING_ITEM && header.type != PacketItemType::WSTRING_ITEM) 
		return std::wstring();

	char* strbuf = new char[header.dataLen + 2];
	memset(strbuf, 0, header.dataLen + 2);
	memcpy(strbuf, data, header.dataLen);
	wstring str((wchar_t*)strbuf);
	delete strbuf;
	return str;
}

BOOL PacketItem::GetAsByteArray(LPBYTE _buf, DWORD _buflen)
{
	if (!_buf || !data || header.dataLen > _buflen || header.type != PacketItemType::BYTE_ARRAY)
		return FALSE;
	memcpy(_buf, data, header.dataLen);
	return TRUE;
}

PacketItem* PacketItem::FromBytes(LPBYTE _buf)
{
	if (_buf == NULL) return NULL;
	DWORD sign = *(DWORD*)_buf;
	if (sign != ITEM_SIGN) return NULL;
	
	PACKETITEMHEADER header;
	memcpy(&header, _buf, sizeof(PACKETITEMHEADER));

	char* namebuf = new char[header.nameLen + 1];
	memset(namebuf, 0, header.nameLen + 1);
	memcpy(namebuf, _buf + sizeof(PACKETITEMHEADER), header.nameLen);
	std::string name = std::string(namebuf);

	LPBYTE data = _buf + sizeof(PACKETITEMHEADER) + header.nameLen;

	switch (header.type) {
		case PacketItemType::BYTE_ARRAY: {
			return new PacketItem(name, data, header.dataLen);
			break;
		}
		case PacketItemType::LONG_ITEM: {
			return new PacketItem(name, *((INT64*)data));
			break;
		}
		case PacketItemType::BOOLEAN_ITEM: {
			return new PacketItem(name, *((bool*)data));
			break;
		}
		case PacketItemType::INT_ITEM: {
			return new PacketItem(name, *((int*)data));
			break;
		}
		case PacketItemType::WSTRING_ITEM: {
			LPBYTE temp = (LPBYTE)GlobalAlloc(GPTR, header.dataLen + 1);
			if (temp) {
				memcpy(temp, data, header.dataLen);
				wstring real((wchar_t*)temp);
				PacketItem* newItem = new PacketItem(name, real);
				GlobalFree(temp);
				return newItem;
			}
		}

		case PacketItemType::STRING_ITEM: {
			LPBYTE temp = (LPBYTE)GlobalAlloc(GPTR, header.dataLen + 2);
			if (temp) {
				memcpy(temp, data, header.dataLen);
				wstring real((wchar_t*)temp);

				PacketItem* newItem = new PacketItem(name, utf8_encode(real));
				GlobalFree(temp);
				return newItem;
			}
			break;
		}
		default: {
			return NULL;
		}
	}

	return NULL;
}
/*
*  (sign + totallen + namelen + datalen + type) + name(ascii) + data(unicode)
*  --Len(sizeof(Ptype) + len(data) = int)--PType(int)--Data(bytes)--
*/
BOOL PacketItem::ToBytes(LPBYTE buf, DWORD buf_len)
{
	if (buf_len < header.totalLen || buf == NULL) return FALSE;
	//copy header
	DWORD copiedBytes = 0;
	
	header.signature = ITEM_SIGN;
	header.totalLen = sizeof(PACKETITEMHEADER) + header.nameLen + header.dataLen;

	memcpy(buf + copiedBytes, &header, sizeof(PACKETITEMHEADER));
	copiedBytes += sizeof(PACKETITEMHEADER);
	//copy name
	DWORD namelen = (int)name.length();
	memcpy(buf + copiedBytes, name.c_str(), namelen);
	copiedBytes += namelen;
	//copy value
	memcpy(buf + copiedBytes, data, header.dataLen);
	return TRUE;
}

PacketItem::~PacketItem()
{
	if (data) GlobalFree(data);
}
