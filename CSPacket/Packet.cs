using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;

namespace SharedPacket
{
    public enum PackItemType
    {
        NONE = 0,
        BYTE_ARRAY = 0x3030,
        STRING_ITEM = 0x4040,
        WSTRING_ITEM = 0x8080,
        LONG_ITEM = 0x5050,
        INT_ITEM = 0x7070,
        BOOLEAN_ITEM = 0x6060,
    };

    public class PackItem
    {
        public const Int32 ITEM_SIGN = 0x62656172;  //bear
        public string name { set; get; }
        protected PackItemType pType { set; get; }
        protected byte[] data { set; get; } = null;

        public PackItem(string _name, bool val)
        {
            name = _name;
            pType = PackItemType.BOOLEAN_ITEM;
            data = BitConverter.GetBytes(val);
        }
        public PackItem(string _name, Int32 val)
        {
            name = _name;
            pType = PackItemType.INT_ITEM;
            data = BitConverter.GetBytes((Int64)val);
        }
        public PackItem(string _name, Int64 val)
        {
            name = _name;
            pType = PackItemType.LONG_ITEM;
            data = BitConverter.GetBytes(val);
        }
        public PackItem(string _name, string val)
        {
            name = _name;
            pType = PackItemType.WSTRING_ITEM;
            data = new UnicodeEncoding().GetBytes(val);
        }
        public PackItem(string _name, byte[] val)
        {
            name = _name;
            pType = PackItemType.BYTE_ARRAY;
            data = val;
        }
        public bool isValid
        {
            get
            {
                if (pType == PackItemType.BOOLEAN_ITEM ||
                    pType == PackItemType.LONG_ITEM ||
                    pType == PackItemType.INT_ITEM ||
                    pType == PackItemType.STRING_ITEM ||
                    pType == PackItemType.WSTRING_ITEM ||
                    pType == PackItemType.BYTE_ARRAY)
                    return true;
                return false;
            }
        }
        public object GetValue()
        {
            if (!isValid || data == null)
                return null;
            switch (pType)
            {
                case PackItemType.BOOLEAN_ITEM:
                    return BitConverter.ToBoolean(data, 0);
                case PackItemType.INT_ITEM:
                case PackItemType.LONG_ITEM:
                    return BitConverter.ToInt64(data, 0);
                case PackItemType.STRING_ITEM:
                case PackItemType.WSTRING_ITEM:
                    return new UnicodeEncoding().GetString(data);
                case PackItemType.BYTE_ARRAY:
                    return data;
            }
            return null;
        }

        public bool GetAsBool()
        {
            object val = GetValue();
            if (pType != PackItemType.BOOLEAN_ITEM || val == null) return false;
            return (bool)val;
        }
        public Int64 GetAsInt64()
        {
            object val = GetValue();
            if (pType != PackItemType.INT_ITEM && pType != PackItemType.LONG_ITEM || val == null)
                return 0;
            return (Int64)val;
        }
        public Int32 GetAsInt()
        {
            object val = GetValue();
            if (pType != PackItemType.INT_ITEM && pType != PackItemType.LONG_ITEM || val == null)
                return 0;
            return (Int32)val;
        }
        public string GetAsString()
        {
            object val = GetValue();
            if (pType != PackItemType.WSTRING_ITEM && pType != PackItemType.STRING_ITEM || val == null)
                return string.Empty;
            return (string)val;
        }
        public byte[] GetAsByte()
        {
            object val = GetValue();
            if (pType != PackItemType.BYTE_ARRAY || val == null)
                return null;
            return (byte[])val;
        }

        /*
         * (sign + totallen + namelen + datalen + type) + name(ascii) + data(unicode)
         *  --Len(sizeof(Ptype) + len(data) = int)--PType(int)--Data(bytes)--
         */
        public byte[] ToBytes()
        {
            if (pType == PackItemType.NONE || data == null || string.IsNullOrEmpty(name))
                return null;

            MemoryStream ms = new MemoryStream();
            byte[] nameBuf = new ASCIIEncoding().GetBytes(name);
            //sign
            ms.Write(BitConverter.GetBytes(ITEM_SIGN), 0, sizeof(Int32));

            //totalLen - int
            int toatalLen = data.Length + nameBuf.Length + sizeof(int) * 5;
            ms.Write(BitConverter.GetBytes(toatalLen), 0, sizeof(int));

            //nameLen - int
            int namelen = nameBuf.Length;
            ms.Write(BitConverter.GetBytes(namelen), 0, sizeof(int));//namelen

            //datalen - int
            int datalen = data.Length;
            ms.Write(BitConverter.GetBytes(datalen), 0, sizeof(int));//namelen
                                                                     //type
            ms.Write(BitConverter.GetBytes((Int32)pType), 0, sizeof(int));//namelen

            //name - n
            ms.Write(nameBuf, 0, nameBuf.Length);//name
                                                 //data - n
            ms.Write(data, 0, data.Length);
            byte[] ret = ms.ToArray();
            ms.Dispose();
            return ret;
        }

        public static PackItem FromBytes(byte[] data)
        {
            if (data.Length < sizeof(int) * 5 + 1) return null;

            MemoryStream ms = new MemoryStream();
            int offset = 0, readLen = 0, totalLen = 0, nameLen = 0, dataLen = 0, pType = 0;
            //sign
            readLen = sizeof(int);
            ms.Write(data, offset, readLen);
            int sign = BitConverter.ToInt32(ms.ToArray(), 0);
            if (sign != ITEM_SIGN)
                return null;
            ms.Dispose();
            offset += readLen;

            //totalLen
            ms = new MemoryStream();
            readLen = sizeof(int);
            ms.Write(data, offset, readLen);
            totalLen = BitConverter.ToInt32(ms.ToArray(), 0);
            if (totalLen < data.Length)
                return null;
            ms.Dispose();
            offset += readLen;

            //nameLen
            ms = new MemoryStream();
            readLen = sizeof(int);
            ms.Write(data, offset, readLen);
            nameLen = BitConverter.ToInt32(ms.ToArray(), 0);
            if (nameLen > totalLen - sizeof(int) * 5)
                return null;
            ms.Dispose();
            offset += readLen;

            //dataLen
            ms = new MemoryStream();
            readLen = sizeof(int);
            ms.Write(data, offset, readLen);
            dataLen = BitConverter.ToInt32(ms.ToArray(), 0);
            if (dataLen != totalLen - sizeof(int) * 5 - nameLen)
                return null;
            ms.Dispose();
            offset += readLen;

            //type
            ms = new MemoryStream();
            readLen = sizeof(int);
            ms.Write(data, offset, readLen);
            pType = BitConverter.ToInt32(ms.ToArray(), 0);
            ms.Dispose();
            offset += readLen;

            //name
            ms = new MemoryStream();
            readLen = nameLen;
            ms.Write(data, offset, nameLen);
            string name = new ASCIIEncoding().GetString(ms.ToArray());
            ms.Dispose();
            offset += readLen;

            //data
            ms = new MemoryStream();
            readLen = dataLen;
            ms.Write(data, offset, dataLen);
            byte[] remainData = ms.ToArray();
            ms.Dispose();
            offset += readLen;

            PackItemType type = (PackItemType)pType;

            switch (type)
            {
                case PackItemType.BOOLEAN_ITEM:
                    Int64 val = BitConverter.ToInt64(remainData, 0);
                    return new PackItem(name, val == 1);
                case PackItemType.INT_ITEM:
                    return new PackItem(name, BitConverter.ToInt64(remainData, 0));
                case PackItemType.LONG_ITEM:
                    return new PackItem(name, BitConverter.ToInt64(remainData, 0));
                case PackItemType.STRING_ITEM:
                case PackItemType.WSTRING_ITEM:
                    return new PackItem(name, new UnicodeEncoding().GetString(remainData, 0, remainData.Length));
                case PackItemType.BYTE_ARRAY:
                    return new PackItem(name, remainData);
            }
            return null;
        }

        public int GetTotalLen()
        {
            if (pType != PackItemType.NONE)
            {
                return data.Length + name.Length + sizeof(int) * 5;
            }
            return 0;
        }
    }

    public class Packet
    {
        public const string TYPE_FILED = "packet_type"; //should match c version
        public const string ID_FILED = "client_id";     //should match c version
        Dictionary<string, PackItem> itemlist { set; get; } = new Dictionary<string, PackItem>();
        public static Packet FromBytes(byte[] _data)
        {
            if (_data.Length < sizeof(long)) return null;

            MemoryStream ms = new MemoryStream();
            //totalLen - packet totalLen
            ms.Write(_data, 0, sizeof(int)); //long - 8bbyte in c#, 4byte in C..
            int totalLen = BitConverter.ToInt32(ms.GetBuffer(), 0);
            if (totalLen > _data.Length)
                return null;
            //itemlist
            Packet packet = new Packet();
            int pos = sizeof(int);
            while (pos < totalLen)
            {
                ms.Dispose();
                ms = new MemoryStream();
                //sign + totallen
                ms.Write(_data, pos + sizeof(int), sizeof(int));
                int itemLen = BitConverter.ToInt32(ms.ToArray(), 0);
                if (itemLen > totalLen - pos)
                    return null;

                byte[] packData = new byte[itemLen];
                Array.Copy(_data, pos, packData, 0, itemLen);
                pos += itemLen;

                PackItem item = PackItem.FromBytes(packData);
                if (item == null || string.IsNullOrEmpty(item.name))
                    continue;
                packet.SetItem(item.name, item);
            }
            return packet;
        }
        public int GetTotalLen()
        {
            int totalLen = sizeof(int);
            foreach (var item in itemlist)
            {
                totalLen += item.Value.GetTotalLen();
            }
            return totalLen;
        }
        public byte[] ToBytes()
        {
            MemoryStream ms = new MemoryStream();
            foreach (var item in itemlist)
            {
                byte[] data = item.Value.ToBytes();
                ms.Write(data, 0, data.Length);//data
            }
            
            //totalbytes
            MemoryStream ms1 = new MemoryStream();
            int totallen = GetTotalLen();
            ms1.Write(BitConverter.GetBytes(totallen), 0, sizeof(int));
            
            //data
            byte[] buf = ms.ToArray();
            ms1.Write(buf, 0, buf.Length);
            
            byte[] ret = ms1.ToArray();

            ms.Dispose();
            ms1.Dispose();
            return ret;
        }
        public bool SetItem(string name, PackItem val)
        {
            itemlist[name] = val;
            return true;
        }
        public bool SetItem(string name, bool val)
        {
            itemlist[name] = new PackItem(name, val);
            return true;
        }
        public bool SetItem(string name, int val)
        {
            PackItem p = new PackItem(name, val);
            int len = p.GetTotalLen();
            itemlist[name] = p;
            return true;
        }
        
        public bool SetItem(string name, Int64 val)
        {
            itemlist[name] = new PackItem(name, val);
            return true;
        }
        public bool SetItem(string name, string val)
        {
            itemlist[name] = new PackItem(name, val);
            return true;
        }
        public bool SetItem(string name, byte[] val)
        {
            itemlist[name] = new PackItem(name, val);
            return true;
        }




        public bool GetAsBool(string name)
        {
            if (itemlist[name] == null) return false;
            return itemlist[name].GetAsBool();
        }
        public int GetAsInt(string name)
        {
            if (itemlist[name] == null) return 0;
            return (int)itemlist[name].GetAsInt64();
        }
        public Int64 GetAsInt64(string name)
        {
            if (itemlist[name] == null) return 0;
            return itemlist[name].GetAsInt64();
        }
        public string GetAsString(string name)
        {
            if (itemlist[name] == null) return string.Empty;
            return itemlist[name].GetAsString();
        }
        public byte[] GetAsBytes(string name)
        {
            if (itemlist[name] == null) return null;
            return itemlist[name].GetAsByte();
        }
    }
}
