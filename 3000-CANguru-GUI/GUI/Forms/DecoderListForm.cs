using CANguru.Controllers;
using CANguru.GUI.Forms;
using CANguru.Models;
using System;
using System.Text;
using System.Windows.Forms;

public partial class DecoderListForm : Form
{
    private readonly DecoderStorage storage;

    public DecoderListForm(DecoderStorage storage)
    {
        InitializeComponent();
        this.storage = storage;

        LoadDecoderList();
    }

    public void LoadDecoderList()
    {
        lvDecoders.Items.Clear();

        foreach (var dec in storage.Decoders.Values.OrderBy(d => d.Name))
        {
            var (type, mac, id, uid, details) = dec.ToListViewColumns();

            var item = new ListViewItem(type);
            item.SubItems.Add(mac);
            item.SubItems.Add(id);
            item.SubItems.Add(uid);
            item.SubItems.Add(details);

            lvDecoders.Items.Add(item);
        }
    }

    public void SaveToFile(string path)
    {
        using (var writer = new StreamWriter(path, false, Encoding.UTF8))
        {
            string line = "Liste der installierten CANguru-Decoder:";
            writer.WriteLine(line);
            line = lvDecoders.Items.Count.ToString() + " Decoder gefunden\r\n";
            writer.WriteLine(line);
            int cnt = 0;
            foreach (ListViewItem item in lvDecoders.Items)
            {
                cnt++;
                line = $"Decoder #{cnt}:\r\n";
                // Erste Spalte
                line += "\tType: " + item.Text + "\r\n";

                // Alle SubItems anhängen
                for (int i = 1; i < item.SubItems.Count; i++)
                {
                    switch (i)
                    {
                        case 1: line += "\tMAC-Adresse: " + item.SubItems[i].Text; break; // MAC
                        case 2: line += "\tID: " + item.SubItems[i].Text; break; // ID
                        case 3: line += "\tUID: " + item.SubItems[i].Text; break; // UID
                        case 4: line += "\tDetails: " + item.SubItems[i].Text; break; // Details
                    }
                    line += "\r\n";
                }

                writer.WriteLine(line);
            }
        }
    }

    private void btnSaveDecoderList_Click(object sender, EventArgs e)
    {
        SaveToFile("DecoderListe.txt");
        MessageBox.Show("Decoder-Liste wurde gespeichert.", "Erfolg",
            MessageBoxButtons.OK, MessageBoxIcon.Information);
    }

    private void btnOK_Click(object sender, EventArgs e)
    {
        this.DialogResult = DialogResult.OK;
        this.Hide();
    }
}
