using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using CANguru.Models;

namespace CANguru.Views
{
    public partial class ManualLokDialog : Form
    {
        public CANguruLok ResultLok { get; private set; }

        public ManualLokDialog()
        {
            InitializeComponent();
            comboDecoderType.Items.Add("MM1");
            comboDecoderType.Items.Add("MM2");
            comboDecoderType.SelectedIndex = 1; // Standard: MM2
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(textName.Text))
            {
                MessageBox.Show("Bitte einen Namen eingeben.");
                return;
            }

            if (comboDecoderType.SelectedIndex < 0)
            {
                MessageBox.Show("Bitte einen Decoder-Typ auswählen.");
                return;
            }

            byte sid = (byte)numericSID.Value;
            string decoder = comboDecoderType.SelectedItem.ToString();

            // künstliche UID erzeugen
            uint uid = MakeManualUidUInt(sid, decoder);

            // MfxUID = nur die Adresse
            byte[] mfxUid = new byte[] { sid };

            ResultLok = new CANguruLok
            {
                Name = textName.Text.Trim(),
                MfxSID = sid,
                DecoderType = decoder,
                IsManual = true,
                UID = uid,
                MfxUID = mfxUid
            };

            DialogResult = DialogResult.OK;
            Close();
        }

        private uint MakeManualUidUInt(byte sid, string decoderType)
        {
            byte proto = decoderType == "MM1" ? (byte)0x01 : (byte)0x02;

            return (uint)(0xFF << 24 | proto << 16 | 0x00 << 8 | sid);
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }
    }
}
