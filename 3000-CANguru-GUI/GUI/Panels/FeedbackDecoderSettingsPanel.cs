using System;
using System.Windows.Forms;

namespace CANguru.GUI.Panels
{
    public partial class FeedbackDecoderSettingsPanel : UserControl
    {
        public FeedbackDecoderSettingsPanel()
        {
            InitializeComponent();
        }

        public void LoadSettings(byte[] payload)
        {
            for (int i = 0; i < 16; i++)
                numK[i].Value = payload[i];
        }

        public byte[] BuildPayload()
        {
            byte[] p = new byte[16];

            for (int i = 0; i < 16; i++)
                p[i] = (byte)numK[i].Value;

            return p;
        }
    }
}
