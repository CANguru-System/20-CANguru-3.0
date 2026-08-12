using CANguru.Controllers;
using CANguru.Models;
using Microsoft.VisualBasic.Logging;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using static CANguru.Controllers.DecoderStorage;
using CANguru.Models;

namespace CANguru.GUI.Forms
{
    public partial class LokListForm : Form
    {
        public LokListForm()
        {
            InitializeComponent();
        }

        public void RefreshLokList(IEnumerable<CANguruLok> loks)
        {
            listViewLoks.Items.Clear();

            int cnt = 0;

            foreach (var lok in loks)
            {
                /*               
                string name = lok.Name.PadRight(20);
                string adr = lok.MfxSID.ToString("X2").PadRight(4);
                string typ = (lok.IsManual ? lok.DecoderType : "mfx").PadRight(6);
                string uid = lok.UID.ToString("X8").PadRight(10);

*/
                cnt++;
                // Nummer in Spalte 0
                var item = new ListViewItem(cnt.ToString());
                // Name in Spalte 1
                item.SubItems.Add(lok.Name);
                // MfxSID in Spalte 2
                item.SubItems.Add(lok.MfxSID.ToString());
                // DecoderType in Spalte 3
                item.SubItems.Add((lok.IsManual ? lok.DecoderType : "mfx"));
                // UID in Spalte 4
                item.SubItems.Add(lok.UID.ToString("X8"));

                listViewLoks.Items.Add(item);
            }
        }

        public void SaveLokListView(string path)
        {
            using (var writer = new StreamWriter(path, false, Encoding.UTF8))
            {
                foreach (ListViewItem item in listViewLoks.Items)
                {
                    string line = $"Lok {item.Text}:\r\n";

                    for (int i = 1; i < item.SubItems.Count; i++)
                    {
                        switch (i)
                        {
                            case 1: line += "Name: " + item.SubItems[i].Text; break; // Name
                            case 2: line += "Adresse: " + item.SubItems[i].Text; break; // Adresse
                            case 3: line += "UID: " + item.SubItems[i].Text; break; // UID
                            case 4: line += "Typ: " + item.SubItems[i].Text; break; // Typ
                        }
                        line += "\r\n";
                    }
                    writer.WriteLine(line);
                }
            }
        }

        private void btnSaveLokList_Click(object sender, EventArgs e)
        {
            SaveLokListView("LokListe.txt");
            MessageBox.Show("Lok-Liste wurde gespeichert.", "Erfolg",
                MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }
    }
}
