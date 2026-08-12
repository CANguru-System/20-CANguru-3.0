using CANguru.GUI.Forms;
using System;
using System.Diagnostics;
using System.Windows.Forms;

namespace CANguru.GUI.Forms
{
    public partial class FeedbackDecoderSettingsForm : Form
    {
        private readonly byte assignedId;

        public FeedbackDecoderSettingsForm(byte decoderId)
        {
            InitializeComponent();
            assignedId = decoderId;
        }

        private void FeedbackDecoderSettingsForm_Load(object sender, EventArgs e)
        {
            // Werte vom Decoder laden
            RequestFeedbackSettings();
        }

        public void HandleGetSettingsAck(byte featureId, byte commandId, byte[] payload)
        {
            if (payload == null || payload.Length != 16)
                return;

            if (this.InvokeRequired)
            {
                this.BeginInvoke(new Action(() =>
                {
                    feedbackPanel.LoadSettings(payload);
                }));
            }
            else
            {
                feedbackPanel.LoadSettings(payload);
            }
        }
        public void HandleSetSettingsAck(byte featureId, byte commandId, byte[] payload)
        {
            if (this.InvokeRequired)
            {
                this.BeginInvoke(new Action(() =>
                {
                    MessageBox.Show("Werte erfolgreich gespeichert.");
                }));
            }
            else
            {
                MessageBox.Show("Werte erfolgreich gespeichert.");
            }
        }

        private void RequestFeedbackSettings()
        {
            // GUI → Bridge → Decoder
            SendFeedbackCommand(ProtocolConstants.FEEDBACK_CMD_GET_SETTINGS);
        }

        private void SendFeedbackCommand(byte command, params byte[] payload)
        {
            var fullPayload = new List<byte>();
            fullPayload.Add(assignedId);
            fullPayload.AddRange(payload);

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_FEEDBACK,
                command,
                fullPayload.ToArray()
            );

            UdpClientSingleton.Instance.Send(packet);
        }
        private void btnSave_Click(object sender, EventArgs e)
        {
            // Panel → Payload bauen
            byte[] payload = feedbackPanel.BuildPayload();

            // GUI → Bridge → Decoder
            SendFeedbackCommand(
                ProtocolConstants.FEEDBACK_CMD_SET_SETTINGS,
                payload
            );

            // Falls du SET_SETTINGS_ACK nutzt:
            // → Erfolgsmeldung kommt später in HandleSetSettingsAck()

            // Falls du KEIN ACK nutzt:
            // MessageBox.Show("Werte gespeichert.");
        }
        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
        private void btnReset_Click(object sender, EventArgs e)
        {
            // Defaultwerte direkt ins Panel schreiben
            feedbackPanel.LoadSettings(new byte[]
            {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
            });
        }
    }
}
