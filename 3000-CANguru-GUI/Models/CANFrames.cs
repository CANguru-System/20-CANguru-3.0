using System;
using System.Collections.Generic;
using System.Text;

namespace CANguru
{
    public class CANFrames
    {
        public byte[] CAN_PING;
        public byte[] magicStart0;
        public byte[] magicStart1;
        public byte[] Neuanmeldezaehler;
        public byte[] GLEISBOX_ALL_PROTO_ENABLE;
        public byte[] SysGO;
        public byte[] SysSTOPP;
        public byte[] LokFunction;
        public byte[] ReadConfig;
        public byte[] M_BIND;
        public byte[] M_VERIFY;
        public byte[] M_DISCOVERY;
        public byte[] FINISHCONFIG;
        public byte[] lastmfxUID;

        public byte newSubscriptionCounter = 1;
        public byte nextLokAddress = 1;
        // ...
        /*
        * CAN-Befehle (Märklin)
        */
        public const byte SYS_CMD = ProtocolConstants.Sys_CMD;    //Systembefehle
        public const byte SYS_CMD_R = 0x01;    //Systembefehle Antwort
        public const byte SYS_STOPP_CMD = 0x00;   //System - Stopp
        public const byte SYS_GO_CMD = 0x01;  //System - Go
        public const byte Gleis_Prot_CMD = 0x08;  //System - Go
        public const byte NeuAnMelder_CMD = 0x09;  //System - Go
        public const byte LokDiscovery_CMD = 0x02;
        public const byte LokDiscovery_CMD_R = 0x03;
        public const byte Lok_Zyklus_Stopp_CMD = 0x04;
        public const byte MFXVerify_CMD = 0x06;
        public const byte MFXVerify_CMD_R = 0x07;
        public const byte Lok_Speed_CMD = 0x08;
        public const byte Lok_Direction_CMD = 0x0A;
        public const byte Lok_Function_CMD = 0x0C;
        public const byte SYS_STAT_CMD = 0x0B;   //System - Status (sendet geänderte Konfiguration)
        public const byte ReadConfig_CMD = 0x0E;
        public const byte ReadConfig_CMD_R = 0x0F;
        public const byte WriteConfig_CMD = 0x10;
        public const byte WriteConfig_CMD_R = 0x11;
        public const byte SWITCH_ACC_CMD = ProtocolConstants.SWITCH_ACC; //Magnetartikel schalten
        public const byte SWITCH_ACC_CMD_R = ProtocolConstants.SWITCH_ACC_R; //Magnetartikel schalten
        public const byte S88_Polling_CMD = 0x20;
        public const byte S88_EVENT_CMD = ProtocolConstants.S88_EVENT;            //Rückmelde-Event
        public const byte S88_EVEN_CMDT_R = ProtocolConstants.S88_EVENT_R;// Rück-Rückmelde-Event
        public const byte PING_CMD = ProtocolConstants.PING;                 //CAN-Teilnehmer anpingen
        public const byte PING_CMD_R = ProtocolConstants.PING_R;           //CAN-Teilnehmer anpingen
        public const byte CONFIG_Status_CMD = ProtocolConstants.CONFIG_Status;
        public const byte CONFIG_Status_CMD_R = ProtocolConstants.CONFIG_Status_R;
        public const byte ConfigDataCompressed_CMD = 0x40;
        public const byte ConfigDataCompressed_CMD_R = 0x41;
        public const byte ConfigDataStream_CMD = 0x42;
        public const byte MfxProc_CMD_R = 0x51;
        //

        public CANFrames()
        {
        }

        public void SetHash(byte high, byte low)
        {
            BuildFrames(high, low);
        }

        public void BuildFrames(byte high, byte low)
        {
            SysSTOPP = new byte[] { 0x00, 0x00, high, low, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
            SysGO = new byte[] { 0x00, 0x00, high, low, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, (byte)'G' };
            GLEISBOX_ALL_PROTO_ENABLE = new byte[] { 0x00, 0x00, high, low, 0x06, 0x00, 0x00, 0x00, 0x00, 0x08, 0x07, 0x00, 0x00, (byte)'G' };
            Neuanmeldezaehler = new byte[] { 0x00, 0x00, high, low, 0x07, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, (byte)'G' };
            M_DISCOVERY = new byte[] { 0x00, 0x02, high, low, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
            M_BIND = new byte[] { 0x00, 0x04, high, low, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
            M_VERIFY = new byte[] { 0x00, 0x06, high, low, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
            LokFunction = new byte[] { 0x00, 0x0c, high, low, 0x06, 0x00, 0x00, 0x40, 0x06, 0x00, 0x01, 0x00, 0x00, (byte)'G' };
            ReadConfig = new byte[] { 0x00, ReadConfig_CMD, high, low, 0x07, 0x00, 0x00, 0x40, 0x00, 0x04, 0x00, 0x00, 0x00, (byte)'G' };
            CAN_PING = new byte[] { 0x00, 0x30, high, low, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
            magicStart0 = new byte[] { 0x00, 0x36, high, low, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
            magicStart1 = new byte[] { 0x00, 0x36, high, low, 0x05, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, (byte)'G' };
            FINISHCONFIG = new byte[] { 0x00, MfxProc_CMD_R, high, low, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,(byte)'G' };
            lastmfxUID =  new byte[] { 0x00, 0x00, 0x00, 0x00 };
        }
    }
}
