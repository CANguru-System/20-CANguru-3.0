using CANguru.GUI.Forms;
using CANguru.GUI.Panels;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing.Imaging;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Threading.Channels;

namespace CANguru.Models
{
    public enum DecoderType
    {
        Signal = ProtocolConstants.DEVTYPE_SIGNAL,
        Switch = ProtocolConstants.DEVTYPE_SWITCH,
        Power = ProtocolConstants.DEVTYPE_POWER,
        Feedback = ProtocolConstants.DEVTYPE_RM
    }

    public abstract class CANguruDevice
    {
        // Von der Bridge vergebene ID
        public byte AssignedId { get; set; }

        // Eindeutige Identität (MAC-Adresse)
        public byte[] Mac { get; set; } = new byte[6];

        // Anzeigename (GUI)
        public string Name { get; set; } = string.Empty;

        // Alive-Überwachung
        public DateTime LastSeen { get; set; } = DateTime.MinValue;

        // READY-Status nach Handshake
        public bool IsReady { get; set; } = false;

        // Protokollversion
        public int ProtocolVersion { get; set; } = 1;

        // Statuswerte (z. B. Ports, Rückmelder, Funktionen)
        public Dictionary<int, int> ChannelStates { get; set; } = new();

    }
    public class CANguruDecoder : CANguruDevice, IDecoderListViewItem
    {
        // Von der Bridge vergebene ID (für WELCOME)
        public byte AssignedId { get; set; }

        // Eindeutige Identität: MAC-Adresse des ESPNOW-Geräts
        public byte[] Mac { get; set; } = new byte[6];

        // Typ des Decoders (Weiche, Signal, Strom, Rückmelder)
        public DecoderType Type { get; set; }

        // Adressbereich, den der Decoder bedient
        public int value0 { get; set; }
        public int value1 { get; set; }
        public int value2 { get; set; }
        public int value3 { get; set; }
        public string Name { get; set; } = string.Empty;
        public bool IsUpdating { get; set; } = false;
        public int OtaProgress { get; set; } = 0;
        public int OtaExpectedAck { get; set; } = -1;

        // Firmware- oder Protokollversion (optional)
        public int ProtocolVersion { get; set; } = 1;


        // READY-Status nach erfolgreichem Handshake
        public DecoderStatus IsReady { get; set; } = DecoderStatus.Lost;

        public void SetMac(params byte[] mac)
        {
            if (mac.Length != 6) throw new ArgumentException("MAC muss 6 Bytes haben");
            for (int i = 0; i < 6; i++)
                Mac[i] = mac[i];
        }

        public string DecoderType { get; set; }
        public uint UID { get; set; }
        public virtual (string Type, string Mac, string Id, string uid, string Details) ToListViewColumns()
        {
            string macStr = $"{Mac[0]:X2}:{Mac[1]:X2}:{Mac[2]:X2}:{Mac[3]:X2}:{Mac[4]:X2}:{Mac[5]:X2}";
            return (DecoderType, macStr, AssignedId.ToString(), UID.ToString(), "");
        }

        public virtual void UpdateFromDecoder()
        {
            // Basis: macht nichts
        }
    }
    public class PowerDecoder : CANguruDecoder
    {
        public ushort MaxCurrent { get; set; }

        public override void UpdateFromDecoder()
        {
            MaxCurrent = DecoderProtocol.ReadUInt16(
                ProtocolConstants.FEATURE_POWER,
                ProtocolConstants.POWER_CMD_GET_THRESHOLD,
                0
            );
        }

        public override (string Type, string Mac, string Id, string uid, string Details) ToListViewColumns()
        {
            string details = $"Max: {MaxCurrent} mA";
            string macStr = $"{Mac[0]:X2}:{Mac[1]:X2}:{Mac[2]:X2}:{Mac[3]:X2}:{Mac[4]:X2}:{Mac[5]:X2}";
            int hexadr = 0xd715 + AssignedId;
            return (Type.ToString(), macStr, AssignedId.ToString(), hexadr.ToString("X4"), details);
        }
    }

    public class SwitchDecoder : CANguruDecoder
    {
        public byte Address { get; set; }
        public int Delay { get; set; }
        public int StepsToEnd { get; set; }
        public override void UpdateFromDecoder()
        {
            var payload = DecoderProtocol.ReadPayload(
                ProtocolConstants.FEATURE_SWITCH,
                ProtocolConstants.SWITCH_CMD_GET_SETTINGS,
                expectedLength: ProtocolConstants.EXP_LNG_SWITCH_SETTINGS,
                requestPayload: new byte[] { (byte)AssignedId }
            );

            if (payload == null)
                return; // Decoder noch nicht bereit

            Address = payload[1];
            Delay = (payload[2] << 8) | payload[3];
            StepsToEnd = (payload[4] << 8) | payload[5];
        }

        public override (string Type, string Mac, string Id, string uid, string Details) ToListViewColumns()
        {
            string details = $"Addr:{Address} Verzögerung:{Delay} Schritte bis Ende:{StepsToEnd}";
            string macStr = $"{Mac[0]:X2}:{Mac[1]:X2}:{Mac[2]:X2}:{Mac[3]:X2}:{Mac[4]:X2}:{Mac[5]:X2}";
            int hexadr = 0xd715 + AssignedId;

            return ("Switch", macStr, AssignedId.ToString(), hexadr.ToString("X4"), details);
        }
    }

    public class SignalDecoder : CANguruDecoder
    {
        public byte Address { get; set; }
        public int Delay { get; set; }
        public int StepsToEnd { get; set; }
        public override void UpdateFromDecoder()
        {
            var payload = DecoderProtocol.ReadPayload(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_GET_SETTINGS,
                expectedLength: ProtocolConstants.EXP_LNG_SIGNAL_SETTINGS  ,
                requestPayload: new byte[] { (byte)AssignedId }
            );

            if (payload == null)
                return; // Decoder noch nicht bereit

            Address = payload[1];
            Delay = (payload[2] << 8) | payload[3];
            StepsToEnd = (payload[4] << 8) | payload[5];
        }
        public override (string Type, string Mac, string Id, string uid, string Details) ToListViewColumns()
        {
            string details = $"Addr:{Address} Verzögerung:{Delay} Schritte bis Ende:{StepsToEnd}";
            string macStr = $"{Mac[0]:X2}:{Mac[1]:X2}:{Mac[2]:X2}:{Mac[3]:X2}:{Mac[4]:X2}:{Mac[5]:X2}";
            int hexadr = 0xd715 + AssignedId;
            return ("Signal", macStr, AssignedId.ToString(), hexadr.ToString("X4"), details);
        }
    }

    public class FeedbackDecoder : CANguruDecoder
    {
        public byte[] Channels = new byte[16];
        public override void UpdateFromDecoder()
        {
            var payload = DecoderProtocol.ReadPayload(
                ProtocolConstants.FEATURE_FEEDBACK,
                ProtocolConstants.FEEDBACK_CMD_GET_SETTINGS,
                expectedLength: ProtocolConstants.EXP_LNG_FEEDBACK_SETTINGS,
                requestPayload: new byte[] { (byte)AssignedId }
            );

            if (payload == null)
                return; // Decoder noch nicht bereit

            // 16 Werte übernehmen
            for (int i = 0; i < 16; i++)
                Channels[i] = payload[i];
        }

        public override (string Type, string Mac, string Id, string uid, string Details) ToListViewColumns()
        {
            string macStr = $"{Mac[0]:X2}:{Mac[1]:X2}:{Mac[2]:X2}:{Mac[3]:X2}:{Mac[4]:X2}:{Mac[5]:X2}";
            int hexadr = 0xd715 + AssignedId;

            string line1 = "K1–K8:  " + string.Join(" ", Channels.Take(8).Select(v => v.ToString("D2")));
            string line2 = "K9–K16: " + string.Join(" ", Channels.Skip(8).Take(8).Select(v => v.ToString("D2")));

            string details = line1 + " | " + line2;

            return ("Feedback", macStr, AssignedId.ToString(), hexadr.ToString("X4"), details);
        }
    }

    public class LokFunktion
    {
        public int Nr { get; set; }      // Funktionsnummer (0–28)
        public int? Typ { get; set; }    // Funktions-Typ (optional)
    }

    public class CANguruLok : CANguruDevice
    {
        // Allgemeine Lokdaten
        public string Name { get; set; } = string.Empty;

        // UID = echte UID (mfx) oder künstliche UID (manuell)
        public uint UID { get; set; }

        // MfxUID:
        // - mfx-Lok: 4 Byte echte UID
        // - manuelle Lok: 1 Byte (Adresse)
        public byte[] MfxUID { get; set; } = Array.Empty<byte>();

        // SID / Adresse
        public byte MfxSID { get; set; }

        // Loktyp
        // mfx-Lok: IsManual = false, DecoderType = ""
        // manuelle Lok: IsManual = true, DecoderType = "MM1" oder "MM2"
        public bool IsManual { get; set; } = false;
        public string DecoderType { get; set; } = string.Empty;
        public Dictionary<int, char> NameFragments { get; set; } = new();
        public int NameFramesReceived { get; set; } = 0;

        public const int MaxNameFrames = 16;

        // CS2-Parameter
        public string Icon { get; set; } = string.Empty;

        public int Av { get; set; } = 60;      // Anfahrverzögerung
        public int Bv { get; set; } = 40;      // Bremsverzögerung
        public int Volume { get; set; } = 100; // Lautstärke
        public int ProgMask { get; set; } = 0x3;
        public int Vmin { get; set; } = 13;
        public int Vmax { get; set; } = 100;
        public int XProt { get; set; } = 2;

        // Funktionsliste (F0–F28 etc.)
        public List<LokFunktion> Funktionen { get; set; } = new();

        /*       public string DetailsString()
               {
                   return $"SpeedSteps:{SpeedSteps}  Functions:{FunctionCount}";
               }
        {name} {adr} {typ} {uid}*/

    }
}
