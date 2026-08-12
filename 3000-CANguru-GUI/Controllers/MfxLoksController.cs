using CANguru;
using Microsoft.VisualBasic.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics.Metrics;
using System.Drawing;
using System.Net.NetworkInformation;
using System.Reflection;
using System.Reflection.Metadata;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Xml.Linq;
using static System.Runtime.InteropServices.JavaScript.JSType;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Rebar;
using CANguru.Models;

namespace CANguru.Controllers
{
    public class CANguruLokRegistry
    {
        private readonly Dictionary<uint, CANguruLok> loks = new();

        public IEnumerable<CANguruLok> AllLoks => loks.Values;

        public bool Contains(uint uid) => loks.ContainsKey(uid);

        public CANguruLok GetOrCreate(uint uid)
        {
            // Prüfen, ob Lok bereits existiert
            if (loks.TryGetValue(uid, out var lok))
                return lok;

            // Neue Lok anlegen
            lok = new CANguruLok
            {
                UID = uid,
            };

            loks[uid] = lok;
            lok.IsManual = false;
            lok.DecoderType = "";
            lok.MfxUID = new byte[4];   // WICHTIG!
            return lok;
        }
    }

    // C#
    public class MfxParser
    {
        private bool bLokDiscovery;
        private bool bWDP_started = false;
        private byte locID;
        private byte newCounter;
        private bool GleisboxFound = false;

        public event Action<string>? LogMessage;
        public event Action<byte[]>? SendFrameRequested;
        public event Action<byte>? incnextLocid;

        private readonly CANFrames canFrames;

        public MfxParser(CANguruLokRegistry registry, CANFrames canFrames)
        {
            this.registry = registry;
            this.canFrames = canFrames;
        }

        private readonly CANguruLokRegistry registry;

        public event Action<CANguruLok>? LokCompleted;


        public MfxParser(CANguruLokRegistry registry)
        {
            this.registry = registry;
        }

        public void setNeuanmeldezaehler()
        {
            // System MFX Neuanmeldezähler setzen
            canFrames.Neuanmeldezaehler[0x0B] = newCounter;
            SendFrameRequested?.Invoke(canFrames.Neuanmeldezaehler);
            LogMessage?.Invoke("Neuanmeldezaehler gesetzt");
        }
        public void ProcessCanFrame(byte[] data)
        {
            byte canId = data[0x01];

            switch (canId)
            {
                case CANFrames.SYS_CMD_R:
                    {
                        byte subCMD = data[0x09];
                        switch (subCMD)
                        {
                            case CANFrames.SYS_STOPP_CMD:
                                // System MFX Neuanmeldezähler setzen
                                setNeuanmeldezaehler();
                                break;
                            case CANFrames.Gleis_Prot_CMD:
                                break;
                            case CANFrames.NeuAnMelder_CMD:
                                break;
                            case CANFrames.SYS_GO_CMD:
                                if (bLokDiscovery == true)
                                {
                                    // Gleisprotokoll frei schalten 
                                    SendFrameRequested?.Invoke(canFrames.GLEISBOX_ALL_PROTO_ENABLE);
                                    // Discovery Mfx anstoßen
                                    SendFrameRequested?.Invoke(canFrames.M_DISCOVERY);
                                    LogMessage?.Invoke("System gestartet");
                                }
                                else
                                {
                                    if (bLokDiscovery == false)
                                    {
                                        bWDP_started = true;
                                        LogMessage?.Invoke("WDP ist gestartet!");
                                    }
                                }
                                break;
                        }
                    }
                    break;
                case CANFrames.LokDiscovery_CMD_R:
                    // mfxdiscovery war erfolgreich
                    if (data[0x04] == 0x05)
                    {
                        bLokDiscovery = true;
                        LogMessage?.Invoke("MFX-Lok gefunden");
                        bindANDverify(data);
                        // an gateway den anfang melden
                    }
                    break;
                case CANFrames.ReadConfig_CMD_R: // ReadConfig_R:
                    if ((data[10] == 0x03) && (bLokDiscovery == true))
                    {
                        HandleMfxNameFrame(data);
                    }
                    break;
                case CANFrames.MfxProc_CMD_R:
                    // wenn die verwendete LocID (content[6]) mit der hier aktuellen LocID
                    // identisch ist, wurde eine neue Lok erkannt
                    bool unkown = newCounter == data[6];
                    // config stream wird beendet
                    //    ConfigStream.finishConfigStruct(unkown);
                    if (unkown)
                    {
                        // LocID um eins erhöhen
                        //        ConfigStream.incnextLocid();
                        // Listbox um die neue Lok ergänzen
                        //        this.lokBox.Invoke(new MethodInvoker(() => ConfigStream.editConfigStruct(lokBox)));
                        // die neue Liste an die Bridge senden
                        //       getANDsendLokCNT();
                    }
                    // Meldung ausgeben
                    // schaltet Strom ein
                    break;
                case CANFrames.PING_CMD_R:
                    gleisboxFound(data[0x0C]);
                    break;
                case CANFrames.MFXVerify_CMD_R:
                    // locID
                    locID = canFrames.M_VERIFY[0x0A];
                    // MFX-UID
                    canFrames.ReadConfig[0x08] = locID;
                    canFrames.ReadConfig[0x09] = 0x04;
                    canFrames.ReadConfig[0x0A] = 0x03;
                    canFrames.ReadConfig[0x0B] = 0x10;
                    // to GLeisbox via Bridge
                    SendFrameRequested?.Invoke(canFrames.ReadConfig);
                    break;
            }
        }

        private void gleisboxFound(byte dev)
        {
            if (GleisboxFound)
                return;
            GleisboxFound = (dev == ProtocolConstants.DEVTYPE_GB);
            if (GleisboxFound)
            {
                //      displayLCD("Gleisbox found!");
                LogMessage?.Invoke("Gleisbox found!");
                LogMessage?.Invoke("Bitte WDP starten!");

            }
            else
            {
                LogMessage?.Invoke("Keine Gleisbox");
                //      displayLCD(" -- No Gleisbox!");
                //      displayLCD(" -- System STOPPED! --");
                //      ESP.restart();
            }
        }
        /*
         * Logik
         * SysStopp
         * SysStopp RE ruft auf: Neuanmeldezaehler und GLEISBOX_ALL_PROTO_ENABLE
         * Neuanmeldezaehler RE ruft auf: SysGO
         * SysGO RE ruft auf M_DISCOVERY
         * M_DISCOVERY RE ruft auf M_BIND und M_VERIFY
         * M_VERIFY RE ruft auf ReadConfig
         * ReadConfig RE (TryFinalizeLokName) ruft auf FINISHCONFIG
         */
        public void handleCANguruLoks(byte nC)
        {
            // Mfx-Anmeldung anstoßen
            newCounter = nC;
            // SysSTOPP Stoppt alle Protokolle → zwingt mfx in Reset
        //    SendFrameRequested?.Invoke(canFrames.SysSTOPP); GLEISBOX_ALL_PROTO_ENABLE
            SendFrameRequested?.Invoke(canFrames.GLEISBOX_ALL_PROTO_ENABLE); 
            bLokDiscovery = true;
            LogMessage?.Invoke("Bitte warten!");
        //    SendFrameRequested?.Invoke(canFrames.SysGO);
        }


        // ----------------------------------------------------
        // Verarbeite CAN-Frames
        // ---------------------------------------------------- 
        // wird für die Erkennung von mfx-Loks gebraucht
        public void bindANDverify(byte[] buffer)
        {
            LogMessage?.Invoke("Lese MFX-Lok aus");
            // MFX-UID
            //Buffer.BlockCopy(source, 0, destination, 0, source.Length);
            Buffer.BlockCopy(buffer, 5, canFrames.lastmfxUID, 0, canFrames.lastmfxUID.Length);
            Buffer.BlockCopy(canFrames.lastmfxUID, 0, canFrames.M_BIND, 5, canFrames.lastmfxUID.Length);
            canFrames.M_BIND[10] = canFrames.nextLokAddress;
            SendFrameRequested?.Invoke(canFrames.M_BIND);
            canFrames.M_VERIFY[10] = canFrames.nextLokAddress;
            // MFX-UID
            Buffer.BlockCopy(canFrames.lastmfxUID, 0, canFrames.M_VERIFY, 5, canFrames.lastmfxUID.Length);
            canFrames.M_VERIFY[0x01] = 0x06;
            SendFrameRequested?.Invoke(canFrames.M_VERIFY);
        }

        private void HandleMfxNameFrame(byte[] frame)
        {
            // UID aus Bytes 5–8
            uint uid = BitConverter.ToUInt32(canFrames.lastmfxUID, 0);


            // Buchstabe steht z.B. in frame[9] (je nach Protokoll)
            char letter = (char)frame[0x0B];
            var lok = registry.GetOrCreate(uid);
            // Framezähler erhöhen
            // mfx liefert Index in 4er-Schritten → echten Zeichenindex berechnen
            int index = frame[0x09];
            lok.NameFramesReceived = (index - 4) / 4;
            if (lok.NameFramesReceived < 0)
                lok.NameFramesReceived = 0;
            //       lok.NameFramesReceived++;
            // Nur echte Buchstaben speichern
            if (letter != '\0')
                lok.NameFragments[lok.NameFramesReceived] = letter;
            // Wenn alle 16 Frames empfangen → finalisieren
            if (lok.NameFramesReceived >= CANguruLok.MaxNameFrames - 1)
                FinalizeLokName(lok);
        }

        private void FinalizeLokName(CANguruLok lok)
        {
            if (lok.NameFragments.Count == 0)
                return;

            var sb = new StringBuilder();

            // Fragmente in Reihenfolge zusammensetzen
            foreach (var kv in lok.NameFragments.OrderBy(k => k.Key))
                sb.Append(kv.Value);
            // Name
            lok.Name = sb.ToString();
            // MfxUID
            // rohe UID-Bytes kopieren
            if (!lok.IsManual)
            {
                if (lok.MfxUID.Length != 4)
                    lok.MfxUID = new byte[4];

                Buffer.BlockCopy(canFrames.lastmfxUID, 0, lok.MfxUID, 0, 4);
            }
            //        Buffer.BlockCopy(canFrames.lastmfxUID, 0, lok.MfxUID, 0, 4);
            // SID
            lok.MfxSID = locID;
            LokCompleted?.Invoke(lok);

            LogMessage?.Invoke("MFX-Lok fertig");
            // an gateway den schluss melden
            bLokDiscovery = false;
            // verwendete locid, damit stellt der Server fest, ob
            // die erkannte Lok neu oder bereits bekannt war
            canFrames.FINISHCONFIG[6] = locID;
            SendFrameRequested?.Invoke(canFrames.FINISHCONFIG);
        }
    }
}
