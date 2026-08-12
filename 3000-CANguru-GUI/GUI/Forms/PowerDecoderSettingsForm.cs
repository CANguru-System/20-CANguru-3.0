using CANguru.GUI.Forms;
using CANguru.GUI.Panels;
using CANguru.Models;
using System;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;

namespace CANguru.GUI.Forms
{
    public partial class PowerDecoderSettingsForm : Form, IDecoderSettingsForm
    {
        private readonly PowerDecoder decoder;

        public PowerDecoderSettingsForm(byte assignedId)
        {
            this.decoder = new PowerDecoder { AssignedId = assignedId };
            InitializeComponent();
            CurrentSettingsFormManager.Register(this);
        }
        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            CurrentSettingsFormManager.Unregister(this);
            base.OnFormClosed(e);
        }

        private void PowerDecoderSettingsForm_Load(object sender, EventArgs e)
        {
            LoadFromDecoder();
        }


        public ushort GetThreshold(ushort defaultValue)
        {
            return DecoderProtocol.ReadUInt16(
                ProtocolConstants.FEATURE_POWER,
                ProtocolConstants.POWER_CMD_GET_THRESHOLD,
                defaultValue
            );
        }

        public void SetThreshold(ushort thr)
        {
            var payload = new byte[]
            {
            (byte)(thr >> 8),
            (byte)(thr & 0xFF)
            };

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_POWER,
                ProtocolConstants.POWER_CMD_SET_THRESHOLD,
                payload
            );

            UdpClientSingleton.Instance.Send(packet);
        }
        private void LoadSettingsIntoUI(params byte[] payload)
        {
            if (payload == null || payload.Length < ProtocolConstants.EXP_LNG_POWER_SETTINGS)
                return;
            ushort thr = (ushort)((payload[0] << 8) | payload[1]);

            this.Invoke(() =>
            {
                panel.numThreshold.Value = thr;
            });
        }

        private void LoadFromDecoder()
        {
            // Beispielwert
            panel.numThreshold.Value = 500;
            Task.Run(() =>
            {
                ushort thr = GetThreshold(2000);
                this.Invoke(() =>
                {
                    panel.numThreshold.Value = thr;
                });
            });

        }

        private void SaveToDecoder()
        {
            int threshold = (int)panel.numThreshold.Value;

            SetThreshold((ushort)threshold);
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            SaveToDecoder();
            this.DialogResult = DialogResult.None; //   .Cancel;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        public void OnResetAck(byte assignedId, byte decoderType)
        {
            // NICHT direkt RequestAndReloadSettings aufrufen
            // sondern asynchron / auf UI-Thread verschieben
            this.BeginInvoke(new Action(() =>
            {
                var payload = DecoderProtocol.RequestAndReloadSettings(assignedId, decoderType);

                if (payload != null)
                    LoadSettingsIntoUI(payload);
                MessageBox.Show(
    "Die Standardeinstellungen wurden zurückgesetzt.",
    "Werkseinstellungen",
    MessageBoxButtons.OK,
    MessageBoxIcon.Warning
);

            }));
        }

        private void btnResetToDefaults_Click(object sender, EventArgs e)
        {
            var request = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SETTING,
                ProtocolConstants.SETTING_CMD_RESET_TO_DEFAULTS,
                new byte[] { decoder.AssignedId }
            );

            UdpClientSingleton.Instance.Send(request);
        }
    }
}
