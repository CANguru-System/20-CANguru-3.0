using System;
using System.Collections.Generic;
using System.Runtime.Intrinsics.Arm;
using System.Text;
using CANguru.Models;

namespace CANguru.Controllers;

public class Cs2LokFileExporter
{
    public int SessionId { get; set; } = 1;      // wird von dir gesetzt
    public int VersionMinor { get; set; } = 3;   // bleibt konstant

    public event Action<string>? LogMessage;

    public void Export(string filePath, IEnumerable<CANguruLok> loks)
    {
        var sb = new StringBuilder();

        // Header
        sb.AppendLine("[lokomotive]");
        sb.AppendLine("version");
        sb.AppendLine(" .minor=3");
        sb.AppendLine("session");
        sb.AppendLine(" .id=43"); // oder dynamisch, wenn du willst

        // Loks sortiert nach Name
        foreach (var lok in loks.OrderBy(l => l.Name))
        {
            WriteLokBlock(sb, lok);
        }

        File.WriteAllText(filePath, sb.ToString(), Encoding.UTF8);
    }

    public void XExport(string filePath, IEnumerable<CANguruLok> loks)
    {
        var sb = new StringBuilder();

        // Header
        sb.AppendLine("[lokomotive]");
        sb.AppendLine("version");
        sb.AppendLine($" .minor={VersionMinor}");
        sb.AppendLine("session");
        sb.AppendLine($" .id={SessionId}");

        foreach (var lok in loks.OrderBy(l => l.Name))
            WriteLokBlock(sb, lok);

        File.WriteAllText(filePath, sb.ToString(), Encoding.UTF8);
    }

    private void WriteLokBlock(StringBuilder sb, CANguruLok lok)
    {
        sb.AppendLine("lokomotive");

        // Name
        sb.AppendLine($" .name={lok.Name}");

        // UID (immer 0xXXXXXXXX)
        sb.AppendLine($" .uid=0x40{lok.MfxSID:X2}");

        // Adresse (hex wie in deinem Beispiel)
        sb.AppendLine($" .adresse=0x{lok.MfxSID:X2}");

        // Typ
        if (lok.IsManual)
        {
            // MM1 / MM2
            sb.AppendLine($" .typ={lok.DecoderType}");
        }
        else
        {
            sb.AppendLine(" .typ=mfx");
        }

        // SID (hex)
        sb.AppendLine($" .sid=0x{lok.MfxSID:X2}");

        // mfxuid
        if (!lok.IsManual && lok.MfxUID != null && lok.MfxUID.Length == 4)
        {
        //    uint mfxUidVal = BitConverter.ToUInt32(lok.MfxUID, 0);
            sb.AppendLine($" .mfxuid=0x{lok.MfxUID[0]:X2}{lok.MfxUID[1]:X2}{lok.MfxUID[2]:X2}{lok.MfxUID[3]:X2}");
        }
        else
        {
            // für manuelle Loks oder fehlende mfxUID
            sb.AppendLine($" .sid=0x{lok.MfxSID:X2}");
        }

        // Icon (falls vorhanden, sonst Name)
        string icon = string.IsNullOrWhiteSpace(lok.Icon) ? lok.Name : lok.Icon;
        sb.AppendLine($" .icon={icon}");

        // Geschwindigkeiten / Lautstärke / Masken – Defaults, falls nicht belegt
        int av = lok.Av > 0 ? lok.Av : 60;
        int bv = lok.Bv > 0 ? lok.Bv : 40;
        int volume = lok.Volume > 0 ? lok.Volume : 100;
        int progMask = lok.ProgMask != 0 ? lok.ProgMask : 0x3;
        int vmin = lok.Vmin > 0 ? lok.Vmin : 13;
        int vmax = lok.Vmax > 0 ? lok.Vmax : 100;
        int xprot = lok.XProt != 0 ? lok.XProt : 2;

        sb.AppendLine($" .av={av}");
        sb.AppendLine($" .bv={bv}");
        sb.AppendLine($" .volume={volume}");
        sb.AppendLine($" .progmask=0x{progMask:X}");
        sb.AppendLine($" .vmin={vmin}");
        sb.AppendLine($" .vmax={vmax}");
        sb.AppendLine($" .xprot={xprot}");

        // Funktionen
        if (lok.Funktionen != null && lok.Funktionen.Count > 0)
        {
            foreach (var f in lok.Funktionen.OrderBy(f => f.Nr))
            {
                sb.AppendLine(" .funktionen");
                sb.AppendLine($" ..nr={f.Nr}");
                if (f.Typ.HasValue)
                    sb.AppendLine($" ..typ={f.Typ.Value}");
            }
        }
        else
        {
            // Falls du wie im Beispiel 0–15 immer schreiben willst:
            for (int i = 0; i <= 15; i++)
            {
                sb.AppendLine(" .funktionen");
                sb.AppendLine($" ..nr={i}");
                // Typ nur setzen, wenn du Defaults willst
                // Beispiel wie in deiner alten Datei:
                if (i == 0) sb.AppendLine(" ..typ=1");
                else if (i >= 1 && i <= 4) sb.AppendLine($" ..typ={50 + i}");
            }
        }

        // Leerzeile zwischen Loks
        // (optional, aber macht die Datei lesbarer)
        // sb.AppendLine();
    }

    private string Escape(string name)
    {
        return name.Replace("\r", "").Replace("\n", "");
    }

    public async Task UploadLokFileToBridge(string bridgeIp, string filePath)
    {
        using var client = new HttpClient();
        using var content = new MultipartFormDataContent();
        using var fileStream = File.OpenRead(filePath);

        content.Add(new StreamContent(fileStream), "file", "lokomotive.cs2");

        var url = $"http://{bridgeIp}/upload/lokomotive.cs2";
        var response = await client.PostAsync(url, content);

        response.EnsureSuccessStatusCode();
    }
}
