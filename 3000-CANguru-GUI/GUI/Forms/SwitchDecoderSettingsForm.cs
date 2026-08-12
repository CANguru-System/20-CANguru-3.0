using CANguru.GUI.Forms;
using CANguru.GUI.Panels;
using CANguru.Models;
using System;
using System.Diagnostics;
using System.Net;
using System.Security.Cryptography;
using System.Windows.Forms;

namespace CANguru.GUI.Forms
{
    public partial class SwitchDecoderSettingsForm : Form, IDecoderSettingsForm
    {
        private readonly SwitchDecoder decoder;
        private enum enum_direction
        {
            to_nullpkt = 0, // Standort vom Motor weg
            to_endpkt,    // Standort zum Motor hin
            any
        };

        private enum enum_position
        {
            nullpkt = 0, // Standort vom Motor weg
            endpkt,    // Standort zum Motor hin
            any
        };

        public SwitchDecoderSettingsForm(byte id)
        {
            InitializeComponent();
            this.decoder = new SwitchDecoder { AssignedId = id };

            CurrentSettingsFormManager.Register(this);

            // Events der switchDecoderSettingsPanel1-Buttons verdrahten
            switchDecoderSettingsPanel1.btnSlowMove2Nullpkt.Click += btnSlowMove2Nullpkt_Click;
            switchDecoderSettingsPanel1.btnSlowMove2End.Click += BtnSlowMove2Endpkt_Click;
            switchDecoderSettingsPanel1.btnSetZero.Click += BtnSetNullpkt_Click;
            switchDecoderSettingsPanel1.btnSetEnd.Click += BtnSetEndpkt_Click;
            switchDecoderSettingsPanel1.btnTest.Click += BtnTestfahrt_Click;
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

        private void SendSwitchCommand(byte command, params byte[] payload)
        {
            // assignedId immer an Position 0 einfügen

            var fullPayload = new List<byte>();
            fullPayload.Add(decoder.AssignedId);   // payload[0]
            fullPayload.AddRange(payload); // payload[1..n]

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SWITCH,
                command,
                fullPayload.ToArray()
            );

            UdpClientSingleton.Instance.Send(packet);
        }

        private void SwitchDecoderSettingsForm_Load(object sender, EventArgs e)
        {
            LoadFromDecoder();
        }

        private void LoadSettingsIntoUI(params byte[] payload)
        {
            if (payload == null || payload.Length < ProtocolConstants.EXP_LNG_SWITCH_SETTINGS)
                return;

            decoder.AssignedId = payload[0];
            switchDecoderSettingsPanel1.numAddress.Value = payload[1];
            int delay = (payload[2] << 8) | payload[3];
            if (delay < switchDecoderSettingsPanel1.numDelay.Minimum)
                delay = (int)switchDecoderSettingsPanel1.numDelay.Minimum;
            else if (delay > switchDecoderSettingsPanel1.numDelay.Maximum)
                delay = (int)switchDecoderSettingsPanel1.numDelay.Maximum;
            switchDecoderSettingsPanel1.numDelay.Value = (ushort)delay;

            int stepsToEnd = (payload[4] << 8) | payload[5];
            if (stepsToEnd < switchDecoderSettingsPanel1.numStepsToEnd.Minimum)
                stepsToEnd = (int)switchDecoderSettingsPanel1.numStepsToEnd.Minimum;
            else if (stepsToEnd > switchDecoderSettingsPanel1.numStepsToEnd.Maximum)
                stepsToEnd = (int)switchDecoderSettingsPanel1.numStepsToEnd.Maximum;
            switchDecoderSettingsPanel1.numStepsToEnd.Value = (ushort)stepsToEnd;
            switchDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = payload[6];
            switchDecoderSettingsPanel1.cmbStepDirection.SelectedIndex = payload[7];
        }

        public void LoadFromDecoder()
        {
            byte[] payload = DecoderProtocol.ReadPayload(
                ProtocolConstants.FEATURE_SWITCH,
                ProtocolConstants.SWITCH_CMD_GET_SETTINGS,
                expectedLength: ProtocolConstants.EXP_LNG_SWITCH_SETTINGS,
                requestPayload: new byte[] { decoder.AssignedId }   // <-- MUSS REIN
            );
            if (payload != null)
                LoadSettingsIntoUI(payload);
        }

        public void SaveToDecoder()
        {
            this.BeginInvoke(new Action(() =>
            {
                var payload = new byte[]
            {
                decoder.AssignedId,
                (byte)switchDecoderSettingsPanel1.numAddress.Value,
                (byte)((ushort)switchDecoderSettingsPanel1.numDelay.Value >> 8),
                (byte)((ushort)switchDecoderSettingsPanel1.numDelay.Value & 0xFF),
                (byte)((ushort)switchDecoderSettingsPanel1.numStepsToEnd.Value >> 8),
                (byte)((ushort)switchDecoderSettingsPanel1.numStepsToEnd.Value & 0xFF),
                (byte)switchDecoderSettingsPanel1.cmbRightLeft.SelectedIndex,
                (byte)switchDecoderSettingsPanel1.cmbStepDirection.SelectedIndex
            };

                var packet = PacketBuilder.Build(
                    ProtocolConstants.FEATURE_SWITCH,
                    ProtocolConstants.SWITCH_CMD_SET_SETTINGS,
                    payload
                );
                UdpClientSingleton.Instance.Send(packet);
            }));
        }

        public void HandleSetCurrPos(int featureId, int commandId, byte[] payload)
        {
            try
            {
                if (featureId != ProtocolConstants.FEATURE_SWITCH ||
                    commandId != ProtocolConstants.SWITCH_CMD_SET_DIR)
                    return;
                if (payload[0] != decoder.AssignedId)
                    return;
                if (payload == null || payload.Length < 2)
                    return;
                this.BeginInvoke(new Action(() =>
                {
                    if (switchDecoderSettingsPanel1 == null || switchDecoderSettingsPanel1.IsDisposed)
                        return;
                    switchDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = payload[1]; // curr_pos
                }));
            }
            catch (Exception ex)
            {
            }
        }


        // SWITCH_CMD_SET_END_ACK
        public void HandleSetEndAck(int featureId, int commandId, byte[] payload)
        {
            try
            {
                if (featureId != ProtocolConstants.FEATURE_SWITCH ||
                    commandId != ProtocolConstants.SWITCH_CMD_SET_END_ACK)
                    return;

                if (payload == null || payload.Length < 2)
                    return;

                int stepsToEnd = (payload[0] << 8) | payload[1];

                if (!this.IsHandleCreated || this.IsDisposed)
                    return;

                this.BeginInvoke(new Action(() =>
                {
                    if (switchDecoderSettingsPanel1 == null || switchDecoderSettingsPanel1.IsDisposed)
                        return;

                    var min = switchDecoderSettingsPanel1.numStepsToEnd.Minimum;
                    var max = switchDecoderSettingsPanel1.numStepsToEnd.Maximum;
                    var clamped = Math.Max(min, Math.Min(max, stepsToEnd));
                    switchDecoderSettingsPanel1.numStepsToEnd.Value = clamped;
                }));
            }
            catch (Exception ex)
            {
            }
            //&            switchDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = 1; // steht jetzt auf Endpunkt
            // SaveToDecoder();
        }

        // SWITCH_CMD_MOVE2START
        // public Button btnSlowMove2Nullpkt;
        // der Stepper läuft langsam vom Motor weg.
        // keine Meldung an die GUI, da die GUI ja die Fahrt initiiert hat und damit weiß, dass sie läuft.
        private void btnSlowMove2Nullpkt_Click(object sender, EventArgs e)
        {
            // 0 = forward, 1 = reverse
            enum_direction direction = enum_direction.to_nullpkt;
            SendSwitchCommand(ProtocolConstants.SWITCH_CMD_MOVE2START, (byte)direction);
        }

        // SWITCH_CMD_SET_ZERO
        // public Button btnSetZero;
        // der Stepper wird am Ende des Laufweges, entfernt vom Stepper, gestoppt,
        // currpos ist jetzt Null. 
        // keine Meldung an die GUI
        private void BtnSetNullpkt_Click(object sender, EventArgs e)
        {
            SendSwitchCommand(ProtocolConstants.SWITCH_CMD_SET_ZERO, Array.Empty<byte>());
        }

        // SWITCH_CMD_SET_END
        // public Button btnSlowMove2End;
        // die Laufrichtung wird umgekehrt
        // der Stepper läuft langsam in Richtung Motor
        // keine Meldung an die GUI
        private void BtnSlowMove2Endpkt_Click(object sender, EventArgs e)
        {
            // 0 = forward, 1 = reverse
            enum_direction direction = enum_direction.to_endpkt;

            SendSwitchCommand(ProtocolConstants.SWITCH_CMD_MOVE2END, (byte)direction);
        }

        // SWITCH_CMD_SET_END
        // public Button btnSetEnd;
        private void BtnSetEndpkt_Click(object sender, EventArgs e)
        {
            SendSwitchCommand(ProtocolConstants.SWITCH_CMD_SET_END, Array.Empty<byte>());
        }

        // SWITCH_CMD_TEST_RUN
        // public Button btnTest;
        private void BtnTestfahrt_Click(object sender, EventArgs e)
        {
            // 0 = forward, 1 = reverse
            enum_direction direction = enum_direction.any;
            if (switchDecoderSettingsPanel1.cmbRightLeft.SelectedIndex == (int) enum_position.nullpkt) // 0 == steht auf Nullpunkt
                direction = enum_direction.to_endpkt;
            else
                direction = enum_direction.to_nullpkt;

            SendSwitchCommand(ProtocolConstants.SWITCH_CMD_TEST_RUN, (byte)direction);
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            SaveToDecoder();
            this.DialogResult = DialogResult.None; //   .Cancel;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            SendSwitchCommand(ProtocolConstants.SWITCH_CMD_STOP_ALL, Array.Empty<byte>());
            this.DialogResult = DialogResult.OK;
            this.Close();
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
