using CANguru.GUI.Forms;
using CANguru.GUI.Panels;
using CANguru.Models;
using System;
using System.Diagnostics;
using System.Windows.Forms;

namespace CANguru.GUI.Forms
{
    public partial class SignalDecoderSettingsForm : Form, IDecoderSettingsForm
    {
        private readonly SignalDecoder decoder;

        private enum enum_position
        {
            nullpkt = 0, // Standort vom Motor weg
            endpkt,    // Standort zum Motor hin
            any
        };

        private enum enum_direction
        {
            to_nullpkt = 0, // Standort vom Motor weg
            to_endpkt,    // Standort zum Motor hin
            any
        };

        public SignalDecoderSettingsForm(byte id)
        {
            InitializeComponent();
            this.decoder = new SignalDecoder { AssignedId = id };

            CurrentSettingsFormManager.Register(this);

            // Events der signalDecoderSettingsPanel1-Buttons verdrahten
            signalDecoderSettingsPanel1.btnSlowMove2Nullpkt.Click += btnSlowMove2Nullpkt_Click;
            signalDecoderSettingsPanel1.btnSlowMove2End.Click += BtnSlowMove2Endpkt_Click;
            signalDecoderSettingsPanel1.btnSetZero.Click += BtnSetNullpkt_Click;
            signalDecoderSettingsPanel1.btnSetEnd.Click += BtnSetEndpkt_Click;
            signalDecoderSettingsPanel1.btnTest.Click += BtnTestfahrt_Click;
        }
        public SignalDecoderSettingsForm()
        {
            InitializeComponent();

            CurrentSettingsFormManager.Register(this);
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            CurrentSettingsFormManager.Unregister(this);
            base.OnFormClosed(e);
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

        public void signalDecoderSettingsPanel1_SettingsChanged(object sender, EventArgs e)
        {
            SaveToDecoder();
        }

        private void SendSignalCommand(byte command, params byte[] payload)
        {
            // assignedId immer an Position 0 einfügen

            var fullPayload = new List<byte>();
            fullPayload.Add(decoder.AssignedId);   // payload[0]
            fullPayload.AddRange(payload); // payload[1..n]

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SIGNAL,
                command,
                fullPayload.ToArray()
            );

            UdpClientSingleton.Instance.Send(packet);
        }

        private void SignalDecoderSettingsForm_Load(object sender, EventArgs e)
        {
            LoadFromDecoder();
        }

        private void LoadSettingsIntoUI(params byte[] payload)
        {
            if (payload == null || payload.Length < ProtocolConstants.EXP_LNG_SIGNAL_SETTINGS)
                return;

            decoder.AssignedId = payload[0];
            signalDecoderSettingsPanel1.numAddress.Value = payload[1];
            int delay = (payload[2] << 8) | payload[3];
            if (delay < signalDecoderSettingsPanel1.numDelay.Minimum)
                delay = (int)signalDecoderSettingsPanel1.numDelay.Minimum;
            else if (delay > signalDecoderSettingsPanel1.numDelay.Maximum)
                delay = (int)signalDecoderSettingsPanel1.numDelay.Maximum;
            signalDecoderSettingsPanel1.numDelay.Value = (ushort)delay;

            int stepsToEnd = (payload[4] << 8) | payload[5];
            if (stepsToEnd < signalDecoderSettingsPanel1.numStepsToEnd.Minimum)
                stepsToEnd = (int)signalDecoderSettingsPanel1.numStepsToEnd.Minimum;
            else if (stepsToEnd > signalDecoderSettingsPanel1.numStepsToEnd.Maximum)
                stepsToEnd = (int)signalDecoderSettingsPanel1.numStepsToEnd.Maximum;
            signalDecoderSettingsPanel1.numStepsToEnd.Value = (ushort)stepsToEnd;
            signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = payload[6];
            signalDecoderSettingsPanel1.cmbStepDirection.SelectedIndex = payload[7];
        }

        public void GetSettings()
        {
            byte[] payload = DecoderProtocol.ReadPayload(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_GET_SETTINGS,
                expectedLength: ProtocolConstants.EXP_LNG_SIGNAL_SETTINGS,
                requestPayload: new byte[] { decoder.AssignedId }   // <-- MUSS REIN
            );
            if (payload != null)
                LoadSettingsIntoUI(payload);
        }

        public void SetSettings()
        {
            this.BeginInvoke(new Action(() =>
            {
                var payload = new byte[]
            {
                decoder.AssignedId,
                (byte)signalDecoderSettingsPanel1.numAddress.Value,
                (byte)((ushort)signalDecoderSettingsPanel1.numDelay.Value >> 8),
                (byte)((ushort)signalDecoderSettingsPanel1.numDelay.Value & 0xFF),
                (byte)((ushort)signalDecoderSettingsPanel1.numStepsToEnd.Value >> 8),
                (byte)((ushort)signalDecoderSettingsPanel1.numStepsToEnd.Value & 0xFF),
                (byte)signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex,
                (byte)signalDecoderSettingsPanel1.cmbStepDirection.SelectedIndex
            };

                var packet = PacketBuilder.Build(
                    ProtocolConstants.FEATURE_SIGNAL,
                    ProtocolConstants.SIGNAL_CMD_SET_SETTINGS,
                    payload
                );
                UdpClientSingleton.Instance.Send(packet);
            }));

        }

        public void LoadFromDecoder()
        {
            GetSettings();
        }

        public void SaveToDecoder()
        {
            SetSettings();
        }

        public void HandleSetCurrPos(int featureId, int commandId, byte[] payload)
        {
            try
            {
                if (featureId != ProtocolConstants.FEATURE_SIGNAL ||
                    commandId != ProtocolConstants.SIGNAL_CMD_SET_DIR)
                    return;
                if (payload[0] != decoder.AssignedId)
                    return;
                if (payload == null || payload.Length < 2)
                    return;
                this.BeginInvoke(new Action(() =>
                {
                    if (signalDecoderSettingsPanel1 == null || signalDecoderSettingsPanel1.IsDisposed)
                        return;
                    signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = payload[1]; // curr_pos
                }));
            }
            catch (Exception ex)
            {
            }
        }


        // SIGNAL_CMD_SET_END_ACK
        public void HandleSetEndAck(int featureId, int commandId, byte[] payload)
        {
            try
            {
                if (featureId != ProtocolConstants.FEATURE_SIGNAL ||
                    commandId != ProtocolConstants.SIGNAL_CMD_SET_END_ACK)
                    return;

                if (payload == null || payload.Length < 2)
                    return;

                int stepsToEnd = (payload[0] << 8) | payload[1];

                if (!this.IsHandleCreated || this.IsDisposed)
                    return;

                this.BeginInvoke(new Action(() =>
                {
                    if (signalDecoderSettingsPanel1 == null || signalDecoderSettingsPanel1.IsDisposed)
                        return;

                    var min = signalDecoderSettingsPanel1.numStepsToEnd.Minimum;
                    var max = signalDecoderSettingsPanel1.numStepsToEnd.Maximum;
                    var clamped = Math.Max(min, Math.Min(max, stepsToEnd));
                    signalDecoderSettingsPanel1.numStepsToEnd.Value = clamped;
                }));
            }
            catch (Exception ex)
            {
            }
            //&            signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = 1; // steht jetzt auf Endpunkt
            SaveToDecoder();
        }

        // SIGNAL_CMD_MOVE2START
        // public Button btnSlowMove2Nullpkt;
        // der Stepper läuft langsam vom Motor weg.
        // keine Meldung an die GUI, da die GUI ja die Fahrt initiiert hat und damit weiß, dass sie läuft.
        private void btnSlowMove2Nullpkt_Click(object sender, EventArgs e)
        {
            // 0 = forward, 1 = reverse
            enum_direction direction = enum_direction.to_nullpkt;
            SendSignalCommand(ProtocolConstants.SIGNAL_CMD_MOVE2START, (byte)direction);
        }

        // SIGNAL_CMD_SET_ZERO
        // public Button btnSetZero;
        // der Stepper wird am Ende des Laufweges, entfernt vom Stepper, gestoppt,
        // currpos ist jetzt Null. 
        // keine Meldung an die GUI
        private void BtnSetNullpkt_Click(object sender, EventArgs e)
        {
            SendSignalCommand(ProtocolConstants.SIGNAL_CMD_SET_ZERO, Array.Empty<byte>());
            /*            signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = 0; // steht jetzt auf Nullpunkt
                        SaveToDecoder();*/
        }

        // SIGNAL_CMD_SET_END
        // public Button btnSlowMove2End;
        // die Laufrichtung wird umgekehrt
        // der Stepper läuft langsam in Richtung Motor
        // keine Meldung an die GUI
        private void BtnSlowMove2Endpkt_Click(object sender, EventArgs e)
        {
            // 0 = forward, 1 = reverse
            enum_direction direction = enum_direction.to_endpkt;

            SendSignalCommand(ProtocolConstants.SIGNAL_CMD_MOVE2END, (byte)direction);
        }

        // SIGNAL_CMD_SET_END
        // public Button btnSetEnd;
        private void BtnSetEndpkt_Click(object sender, EventArgs e)
        {
            SendSignalCommand(ProtocolConstants.SIGNAL_CMD_SET_END, Array.Empty<byte>());
        }

        // SIGNAL_CMD_TEST_RUN
        // public Button btnTest;
        private void BtnTestfahrt_Click(object sender, EventArgs e)
        {
            // 0 = forward, 1 = reverse
            enum_direction direction = enum_direction.any;
            if (signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex == 0) // 0 == steht auf Nullpunkt
                direction = enum_direction.to_endpkt;
            else
                direction = enum_direction.to_nullpkt;

            SendSignalCommand(ProtocolConstants.SIGNAL_CMD_TEST_RUN, (byte)direction);
            /*            if (signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex == 0) // 0 == steht auf Nullpunkt
                            signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = 1;
                        else
                            signalDecoderSettingsPanel1.cmbRightLeft.SelectedIndex = 0;
                        SaveToDecoder();*/
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            SaveToDecoder();
            this.DialogResult = DialogResult.None; //   .Cancel;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            SendSignalCommand(ProtocolConstants.SIGNAL_CMD_STOP_ALL, Array.Empty<byte>());
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
