using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.NetworkInformation;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AsyncPingApp
{
    public partial class Form1 : Form
    {
        // ************************ GLOBAL VARIABLES **************************************************************

        string lightFile = "licht.txt";

        public Form1()
        {
            InitializeComponent();
            if (System.IO.File.Exists(lightFile))
            {
                try
                {
                    // Einlesen der Daten aus der Datei
                    using (StreamReader reader = new StreamReader(lightFile))
                    {
                        string line;
                        while ((line = reader.ReadLine()) != null)
                        {
                            listBoxResults.Items.Add(line);
                        }
                    }
                    txtIP.Text = "Gespeicherte Licht-Adressen eingelesen.";
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Exception: " + ex.Message);
                }
                finally
                {
                    Console.WriteLine("Executing finally block.");
                }
            }
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

            for (int adr = firstAddress; adr < lastAddress; adr++)
            {
                string ip = baseAdress + adr.ToString();
                await PingAsync(ip);
            }
            txtIP.Text = "Fertig!";
            // Überprüfen, ob die ListBox Einträge enthält
            if (listBoxResults.Items.Count > 0)
            {
                // Speichern der Daten in die Datei
                using (StreamWriter writer = new StreamWriter(lightFile))
                {
                    foreach (string item in listBoxResults.Items)
                    {
                        // Zugriff auf jeden Eintrag
                        writer.WriteLine(item);
                    }
                }                
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
                        }
                    }
                    txtIP.Text = ipAddress +" - " + reply.Status;
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

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (listBoxResults.SelectedItem != null)
            {
                string url = listBoxResults.SelectedItem.ToString();
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
        }
    }
}
