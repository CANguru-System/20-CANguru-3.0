using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;

namespace AsyncPingApp
{
    public partial class Form1 : Form
    {
        // ************************ GLOBAL VARIABLES **************************************************************
        //

        public class LightXXListStruct
        {
            public string lightXX { get; set; }
            public string ip { get; set; }
            public LightXXListStruct(string light, string ipadr)
            {
                lightXX = light;
                ip = ipadr;
            }
        }

        string lightFile = "licht.json";
        // Create a dynamic array (List)
        List<LightXXListStruct> LightXXList = new List<LightXXListStruct>();

        public Form1()
        {
            InitializeComponent();

            if (System.IO.File.Exists(lightFile))
            {
                try
                {
                    // JSON-Datei einlesen
                    string jsonContent = File.ReadAllText(lightFile);
                    //
                    // JSON in Liste von Objekten deserialisieren
                    LightXXList = JsonConvert.DeserializeObject<List<LightXXListStruct>>(jsonContent);
                    // Ausgabe der Daten
                    foreach (var oneLight in LightXXList)
                    {
                        listBoxResults.Items.Add(oneLight.lightXX + " - " + oneLight.ip);
                    }
                    listBoxResults.SelectedIndex = listBoxResults.Items.Count - 1;
                }
                catch (Exception ex)
                {
                    txtIP.Text = "Exception: " + ex.Message;
                }
                finally
                {
                    txtIP.Text = "Gespeicherte Licht-Adressen eingelesen.";
                }
            }
            // DoubleClick - Ereignis abonnieren
            listBoxResults.DoubleClick += listBoxResults_DoubleClick;
        }

        private async void btnScan_Click(object sender, EventArgs e)
        {
            string firstAdr = IPAdr0.Text;
            string lastAdr = IPAdr1.Text;
            string[] adrParts0 = firstAdr.Split('.');
            string baseAdress = adrParts0[0] + "." + adrParts0[1] + "." + adrParts0[2] + ".";
            int firstAddress = int.Parse(adrParts0[3]);
            string[] adrParts1 = lastAdr.Split('.');
            int lastAddress = int.Parse(adrParts1[3]); ;
            listBoxResults.Items.Clear();
            LightXXList.Clear();

            for (int adr = firstAddress; adr < lastAddress; adr++)
            {
                string ip = baseAdress + adr.ToString();
                await PingAsync(ip);
            }
            txtIP.Text = "Fertig!";
            // Überprüfen, ob die ListBox Einträge enthält
            if (listBoxResults.Items.Count > 0)
            {
                // Liste serialisieren und in die Datei schreiben

                string json = JsonConvert.SerializeObject(LightXXList);
                File.WriteAllText(lightFile, json);

                txtIP.Text += "Licht-Adressen gespeichert.";
                // Den Index des letzten Eintrags auswählen
                listBoxResults.SelectedIndex = listBoxResults.Items.Count - 1;
            }
        }

        private async Task PingAsync(string ipAddress)
        {
            try
            {
                using (Ping ping = new Ping())
                {
                    PingReply reply = await ping.SendPingAsync(ipAddress, 100);
                    if (reply.Status == IPStatus.Success)
                    {
                        IPHostEntry hostEntry = Dns.GetHostEntry(reply.Address);
                        if (hostEntry.HostName.Contains("LICHT"))
                        {
                            string url = hostEntry.HostName.ToString();
                            int pos = url.IndexOf(".");
                            url = url.Substring(0, pos);
                            //    url += "/" + reply.Address.ToString();
                            listBoxResults.Items.Add(url);
                            listBoxResults.TopIndex = listBoxResults.Items.Count - 1;
                            LightXXList.Add(new LightXXListStruct(url, ipAddress));
                        }
                    }
                    txtIP.Text = ipAddress + " - " + reply.Status;
                }
            }
            catch (Exception ex)
            {
                txtIP.Text = "Fehler bei " + ipAddress + ":" + ex.Message;
            }
        }

        private void btnExit_Click(object sender, EventArgs e)
        {
            // Anwendung beenden
            Application.Exit();
        }

        private void Start_HTML(string url)
        {
            try
            {
                url = "http://" + url + ".local";

                // Standardbrowser mit der URL öffnen
                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = url,
                    UseShellExecute = true
                };

                Process.Start(psi);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Fehler beim Öffnen der URL: {ex.Message}", "Fehler", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (listBoxResults.SelectedItem != null)
            {
                // Den Index des ausgewählten Eintrags abrufen
                int selectedIndex = listBoxResults.SelectedIndex;
                LightXXListStruct onelight = LightXXList[selectedIndex]; // Zugriff auf das 2. Element (Index 1)
                // Aktion ausführen
                Start_HTML(onelight.lightXX);
            }
        }

        private void listBoxResults_DoubleClick(object sender, EventArgs e)
        {
            // Überprüfen, ob ein Element ausgewählt ist
            ListBox listBoxResults = sender as ListBox;
            if (listBoxResults?.SelectedItem != null)
            {
                // Den Index des ausgewählten Eintrags abrufen
                int selectedIndex = listBoxResults.SelectedIndex;
                LightXXListStruct onelight = LightXXList[selectedIndex]; // Zugriff auf das 2. Element (Index 1)
                // Aktion ausführen
                Start_HTML(onelight.lightXX);
            }
        }

        private async void idbtn_Click(object sender, EventArgs e)
        {
            if (listBoxResults.SelectedItem != null)
            {
                // Den Index des ausgewählten Eintrags abrufen
                int selectedIndex = listBoxResults.SelectedIndex;
                LightXXListStruct onelight = LightXXList[selectedIndex]; // Zugriff auf das X. Element (Index X-1)
                // Aktion ausführen
                using (HttpClient client = new HttpClient())
                {
                    client.BaseAddress = new Uri($"http://{onelight.ip}:{80}/");
                    StringContent content = new StringContent("/IDENT", Encoding.UTF8, "text/plain");
                    try
                    {
                        HttpResponseMessage response = await client.PostAsync("IDENT", content);
                        if (response.IsSuccessStatusCode)
                        {
                            txtIP.Text = "Nachricht erfolgreich gesendet!";
                        }
                        else
                        {
                            txtIP.Text = "Fehler: {response.StatusCode}";
                        }
                    }
                    catch (Exception ex)
                    {
                        txtIP.Text = "Fehler: {ex.Message}";
                    }
                }
            }
        }
    }
}

