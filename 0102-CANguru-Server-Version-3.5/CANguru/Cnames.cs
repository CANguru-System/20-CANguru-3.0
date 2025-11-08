using System;

namespace CANguruX
{
    class Cnames
    {
        public const byte cntCS2Files = 1;
        public const byte shortnameLng = 7;
        public const byte shortnameLng2transfer = shortnameLng - 2;
        // init array
        // Dateien mit "0" sind verzichtbar
        static public string path = @"CANguru";
        static public string ininame = @"\CANguru.ini";
        static public string saveTxtname = @"\CANguru.txt";
        static public string cs2name = @"\lokomotive.cs2";
        static public string cs2nameshort = @"loks   ";
        static public string name001 = @"\lokomotive.001";
        static public string name002 = @"\lokomotive.002";
        static public string tmpname = @"\lokomotive.tmp";
        static public string bakname = @"\lokomotive.bak";
        static public string cfgname = @"\lokomotive.cfg";
        public const byte lngFrame = 13;
        public const byte maxCANgurus = 50;
        // der Lichtdecodder hat momentan 10 Zeilen; aber lieber noch einige drauf
        public const byte maxConfigLines = 20;
        // IN is even
        static public Int32 localPortDelta = 2;      // local port to listen on
        static public Int32 portinCAN = 15730 + localPortDelta;
        static public Int32 portoutCAN = 15731 + localPortDelta;
        static public string IP_CAN = "192.168.178.71";
        public const int port = 23;
        public const byte toCAN = 1;
        public const char delimiter = '/';
        public byte[] sep = { 0x25 }; // %
        public byte[] separator()
        {
            return sep;
        }
    }
}
