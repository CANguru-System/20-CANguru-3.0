using CANguru.Models;
using CANguru.Networking;
using Microsoft.VisualBasic.Logging;
using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace CANguru.Controllers
{
    public class DeviceStorage<T> where T : CANguruDevice
    {

        protected readonly List<T> _devices = new();

        public IReadOnlyList<T> Devices => _devices;

        public virtual void AddOrUpdate(T device)
        {
            var existing = _devices.FirstOrDefault(d => d.AssignedId == device.AssignedId);

            if (existing != null)
            {
                // Gemeinsame Felder aktualisieren
                existing.Name = device.Name;
                existing.LastSeen = device.LastSeen;
                existing.IsReady = device.IsReady;
                existing.ProtocolVersion = device.ProtocolVersion;

                // MAC aktualisieren (falls sich etwas ändert)
                for (int i = 0; i < 6; i++)
                    existing.Mac[i] = device.Mac[i];

                // Spezifische Felder aktualisieren
                CopySpecific(existing, device);
            }
            else
            {
                _devices.Add(device);
            }
        }

        // Kann von abgeleiteten Klassen überschrieben werden
        protected virtual void CopySpecific(T target, T source)
        {
            // Default: nichts tun
        }

        public void Remove(int id)
        {
            var d = _devices.FirstOrDefault(x => x.AssignedId == id);
            if (d != null)
                _devices.Remove(d);
        }

        public void Clear() => _devices.Clear();
    }

    public class DecoderStorage : DeviceStorage<CANguruDecoder>
    {

        protected readonly List<CANguruDecoder> _decoders_storage = new();
        public Dictionary<uint, CANguruDecoder> Decoders { get; private set; } = new();


        public void Remove(int AssignedId)
        {
            var d = _decoders_storage.FirstOrDefault(x => x.AssignedId == AssignedId);
            if (d != null)
                _decoders_storage.Remove(d);
        }

        public void Clear()
        {
            _decoders_storage.Clear();
        }

        public void AddOrUpdate(CANguruDecoder decoder)
        {
            uint key = (uint)decoder.AssignedId;

            if (Decoders.TryGetValue(key, out var existing))
            {
                // Update bestehender Eintrag
                existing.Type = decoder.Type;
                existing.value0 = decoder.value0;
                existing.value1 = decoder.value1;
                // (kopiere ggf. weitere Felder)
            }
            else
            {
                // neu anlegen
                Decoders[key] = decoder;
            }

            // Falls Liste verwendet wird, synchronisieren
            _decoders_storage.Clear();
            _decoders_storage.AddRange(Decoders.Values);
        }

        protected override void CopySpecific(CANguruDecoder target, CANguruDecoder source)
        {
            target.Type = source.Type;
            target.value0 = source.value0;
            target.value1 = source.value1;
        }

        public class LokStorage : DeviceStorage<CANguruLok>
        {
            private readonly string filePath;

            protected readonly List<CANguruLok> _loks = new();

            public Dictionary<uint, CANguruLok> Loks { get; private set; } = new();

            // Korrigierter Konstruktorname muss mit dem Klassennamen übereinstimmen.
            public LokStorage(string filePath)
            {
                this.filePath = filePath;
                Load();
            }

            public void Save()
            {
                var json = JsonSerializer.Serialize(Loks, new JsonSerializerOptions
                {
                    WriteIndented = true
                });

                File.WriteAllText(filePath, json);
            }

            public void Load()
            {
                if (!File.Exists(filePath))
                {
                    Loks = new Dictionary<uint, CANguruLok>();
                    return;
                }

                var json = File.ReadAllText(filePath);

                Loks = JsonSerializer.Deserialize<Dictionary<uint, CANguruLok>>(json)
                       ?? new Dictionary<uint, CANguruLok>();
                LoadNextMfxSID();
            }

            public int GetNextMfxSID()
            {
                // Falls keine Loks vorhanden sind → erste Adresse = 1
                if (Loks == null || Loks.Count == 0)
                    return 1;

                // Nur mfx-Loks berücksichtigen (IsManual == false)
                var mfxSids = Loks.Values
                    .Where(l => !l.IsManual)                 // echte mfx-Loks
                    .Select(l => (int)l.MfxSID)              // SID extrahieren
                    .ToList();

                if (mfxSids.Count == 0)
                    return 1; // keine mfx-Loks → erste Adresse

                int max = mfxSids.Max();

                return max + 1;
            }

            public int NextMfxSID { get; private set; } = 1;

            public void LoadNextMfxSID()
            {
                if (!File.Exists(filePath))
                {
                    Loks = new Dictionary<uint, CANguruLok>();
                    NextMfxSID = 1;
                    return;
                }

                var json = File.ReadAllText(filePath);

                Loks = JsonSerializer.Deserialize<Dictionary<uint, CANguruLok>>(json)
                       ?? new Dictionary<uint, CANguruLok>();

                // ⭐ Hier wird die nächste freie SID berechnet
                NextMfxSID = GetNextMfxSID();
            }

            public void AddOrUpdate(CANguruLok lok)
            {
                // 1. UID ist immer der Dictionary-Key
                uint key = lok.UID;

                // 2. MfxUID korrekt initialisieren
                if (lok.IsManual)
                {
                    // manuelle Lok → 1 Byte
                    if (lok.MfxUID == null || lok.MfxUID.Length != 1)
                        lok.MfxUID = new byte[] { lok.MfxSID };
                }
                else
                {
                    // mfx-Lok → 4 Byte
                    if (lok.MfxUID == null || lok.MfxUID.Length != 4)
                        lok.MfxUID = new byte[4];
                }

                // 3. Lok einfügen oder aktualisieren
                if (Loks.TryGetValue(key, out var existing))
                {
                    // aktualisieren
                    existing.Name = lok.Name;
                    existing.MfxSID = lok.MfxSID;
                    existing.DecoderType = lok.DecoderType;
                    existing.IsManual = lok.IsManual;
                    existing.MfxUID = lok.MfxUID;
                    existing.Icon = lok.Icon;
                    existing.Av = lok.Av;
                    existing.Bv = lok.Bv;
                    existing.Volume = lok.Volume;
                    existing.ProgMask = lok.ProgMask;
                    existing.Vmin = lok.Vmin;
                    existing.Vmax = lok.Vmax;
                    existing.XProt = lok.XProt;
                    existing.Funktionen = lok.Funktionen;
                }
                else
                {
                    // neu anlegen
                    Loks[key] = lok;
                }
                Save();
                LoadNextMfxSID();
            }

            public void Remove(uint uid)
            {
                if (Loks.Remove(uid))
                    Save();
            }
            protected override void CopySpecific(CANguruLok target, CANguruLok source)
            {
                target.Name = source.Name;
                target.MfxSID = source.MfxSID;
                target.DecoderType = source.DecoderType;
                target.IsManual = source.IsManual;
                target.MfxUID = source.MfxUID;
                target.Icon = source.Icon;
                target.Av = source.Av;
                target.Bv = source.Bv;
                target.Volume = source.Volume;
                target.ProgMask = source.ProgMask;
                target.Vmin = source.Vmin;
                target.Vmax = source.Vmax;
                target.XProt = source.XProt;
                target.Funktionen = source.Funktionen;
            }

            private readonly List<CANguruLok> loks = new();

            public IReadOnlyList<CANguruLok> GetAll()
            {
                return loks.AsReadOnly();
            }
        }
    }
}
