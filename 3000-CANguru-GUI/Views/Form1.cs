using CANguru;
using CANguru.Communication;
using CANguru.Controllers;
using CANguru.Controls;
using CANguru.GUI.Forms;
using CANguru.GUI.Panels;
using CANguru.Models;
using CANguru.Networking;
using CANguru.Protocol;
using Microsoft.VisualBasic;
using Microsoft.VisualBasic.Logging;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Channels;
using System.Timers;
using System.Windows.Forms;
using System.Xml.Linq;
using static CANguru.Controllers.DecoderStorage;
using static System.Net.Mime.MediaTypeNames;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using Application = System.Windows.Forms.Application;

namespace CANguru.Views
{
    public partial class Form1 : Form
    {
        #region Felder und globale Zustände

        // CAN-Frames / Protokoll
        private CANFrames CANFrames;
        private CANguruLokRegistry lokRegistry = new();
        private MfxParser mfxParser;
        private LokStorage lokstorage;
        private DecoderStorage decoderStorage;
        private DecoderList decoderController;
        private SwitchDecoderSettingsForm activeSwitchForm;
        private SignalDecoderSettingsForm activeSignalForm;
        private FeedbackDecoderSettingsForm activeFeedbackForm;
        private DecoderListForm decoderListForm;
        private LokListForm lokListForm;

        // UDP
        private UdpClient udpReceiver;

        // Firmware-Upload
        private CANguru.OTA.OtaUploader otaUploader;
        private CancellationTokenSource _uploadCts;
        private byte lastPercent = 0;

        /*&&&        private CancellationTokenSource _uploadCts;
                private int blockSize = ProtocolConstants.BlockSizeGUI;
                private int maxRetries = 5;
                private int ackTimeout = 500; // ms
                private uint seq;
        */
        int totalBlocks = 0;

        // lokomotive.cs2 -Upload
        private Cs2FileUploader uploader;

        // Alive-Überwachung
        private DecoderStatus currentDecoderStatus = DecoderStatus.Lost;
        private bool guiClosing = false;
        private bool isFirmwareUploadActive = false;
        private AliveManager aliveManager = new AliveManager();
        private bool firstAliveReceived = true;

        private int lastPower = -1;
        private int lastSwitch = -1;
        private int lastSignal = -1;
        private int lastFeedback = -1;

        // CAN-Queues
        private readonly ConcurrentQueue<(uint seq, byte[] data)> incomingFrames = new();
        private readonly ConcurrentQueue<byte[]> outgoingFrames = new();

        // CANguru-Steuerzustand
        private enum enum_canguruStatus
        {
            idle,
            startGleisbox,
            wait4LokListe,
            systemIsRunning
        }
        private enum_canguruStatus canguruStatus;

        // Einmal-Timer für verzögerten Start
        private System.Windows.Forms.Timer oneShotTimer = new System.Windows.Forms.Timer();

        //
        public PowerDecoderController PowerCtrl = new PowerDecoderController();
        #endregion

        #region Konstruktor und Form-Initialisierung
        //
        public static Form1 Instance { get; private set; }
        // Für Drag-Move
        public const int WM_NCLBUTTONDOWN = 0xA1;
        public const int HTCAPTION = 0x2;
        private VerticalDualProgressBar dualBar;
        private ToggleStopGoButton btnStopGo;

        [DllImport("user32.dll")]
        public static extern bool ReleaseCapture();

        [DllImport("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd, int Msg, int wParam, int lParam);

        // Einmal-Timer für Start der CANguru-Loop
        private void InitOneShotTimer()
        {
            oneShotTimer.Interval = 1000;
            oneShotTimer.Tick += OneShotTimer_Tick;
            oneShotTimer.Start();
        }
        private void OneShotTimer_Tick(object sender, EventArgs e)
        {
            oneShotTimer.Stop();
            oneShotTimer.Dispose();
            StartCANguruLoop();
        }

        private void InitDualBar()
        {
            dualBar = new VerticalDualProgressBar();
            dualBar.Visible = false;
            dualBar.CurrentMaximum = 100;
            dualBar.TotalMaximum = 100;
            dualBar.CurrentValue = 0;
            dualBar.TotalValue = 0;
            dualBar.CurrentColor = Color.LimeGreen;
            dualBar.TotalColor = Color.DodgerBlue;
            dualBar.DividerColor = Color.Black;
            dualBar.TextColor = Color.Black;

            // Position irgendwo sinnvoll platzieren:
            dualBar.Width = 100;
            dualBar.Height = 105;
            dualBar.Left = 500;   // z.B. an Stelle der alten Bars
            dualBar.Top = 450;

            this.Controls.Add(dualBar);
        }
        private void InitStopGoButton()
        {
            btnStopGo = new ToggleStopGoButton();
            btnStopGo.Width = 75;
            btnStopGo.Height = 40;
            btnStopGo.Left = 513;
            btnStopGo.Top = 560;

            // Aktionen verbinden
            btnStopGo.StopClicked += BtnStopGo_StopClicked;
            btnStopGo.GoClicked += BtnStopGo_GoClicked;

            this.Controls.Add(btnStopGo);
            Log("Gleisspannung ausgeschaltet.");
            btnStopGo.SetMode(false);
        }

        public Form1()
        {
            InitializeComponent();
            InitUploader();
            InitOneShotTimer();
            CANFrames = new CANFrames();
            mfxParser = new MfxParser(lokRegistry, CANFrames);
            decoderController = new DecoderList();
            lokListForm = new LokListForm();
            pictureBox1.Image = Icon.ExtractAssociatedIcon(Application.ExecutablePath).ToBitmap();
            GuiLogger.MessageLogged += OnGuiLog;
            // AliveManager initialisieren
            aliveManager = new AliveManager();

            // Hier kommt die Registrierung hin:
            aliveManager.AliveSummaryReceived += (power, sw, signal, feedback) =>
            {
                lastPower = UpdateAliveField(cntPower, lastPower, power);
                lastSwitch = UpdateAliveField(cntSwitch, lastSwitch, sw);
                lastSignal = UpdateAliveField(cntSignal, lastSignal, signal);
                lastFeedback = UpdateAliveField(cntFeedback, lastFeedback, feedback);
            };
            // Parser-Callbacks
            mfxParser.LogMessage += OnParserLog;
            mfxParser.incnextLocid += OnincnextLocid;
            mfxParser.LokCompleted += OnLokCompleted;
            mfxParser.SendFrameRequested += OnSendFrameRequested;

            generateHash();

            canguruStatus = enum_canguruStatus.idle;

            // UI-Initialisierung für UpDowns
            UpDnnewSubscriptionCounter.Minimum = 1;
            UpDnnewSubscriptionCounter.Maximum = 255;
            CANFrames.newSubscriptionCounter = Properties.Settings.Default.newSubscriptionCounter;
            UpDnnewSubscriptionCounter.Value = CANFrames.newSubscriptionCounter;

            UpDnnextLokAddress.Minimum = 1;
            UpDnnextLokAddress.Maximum = 255;
            CANFrames.nextLokAddress = Properties.Settings.Default.nextLokAddress;
            UpDnnextLokAddress.Value = CANFrames.nextLokAddress;

            STOPP_Timer.Enabled = false;

            // Storage initialisieren
            lokstorage = new LokStorage("mfx_loks.json");
            ApplyNextMfxSIDToUI();
            decoderStorage = new DecoderStorage();

            // Fenster / Aussehen

            // Fenster ohne Rahmen
            this.FormBorderStyle = FormBorderStyle.None;
            this.MaximizeBox = false;
            this.MinimizeBox = false;

            // Schatten aktivieren
            this.Padding = new Padding(1);

            radioSingle.Checked = true;

            UpdateLokList();

            // Power
            PowerCtrl.ValuesUpdated += UpdatePowerUI;
            PowerCtrl.ShutdownTriggered += PowerShutdownUI;
            //
            Instance = this;

            // ListView Decoder-Liste
            decoderListForm = new DecoderListForm(decoderStorage);
            // OTA-Firmaware
            otaUploader = new CANguru.OTA.OtaUploader(
                log: text => Log(text),
                setPercent: percent =>
                {
                    dualBar.CurrentValue = percent;
                    dualBar.CurrentText = $"{percent}%";
                    dualBar.Invalidate();
                },
                blockSize: ProtocolConstants.BlockSizeGUI,
                maxRetries: 5,
                ackTimeoutMs: 500
            );
        }
        // Schatten hinzufügen
        protected override CreateParams CreateParams
        {
            get
            {
                const int CS_DROPSHADOW = 0x00020000;
                CreateParams cp = base.CreateParams;
                cp.ClassStyle |= CS_DROPSHADOW;
                return cp;
            }
        }

        // Abgerundete Ecken
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            int radius = 12;
            var path = new System.Drawing.Drawing2D.GraphicsPath();

            path.AddArc(0, 0, radius, radius, 180, 90);
            path.AddArc(this.Width - radius, 0, radius, radius, 270, 90);
            path.AddArc(this.Width - radius, this.Height - radius, radius, radius, 0, 90);
            path.AddArc(0, this.Height - radius, radius, radius, 90, 90);
            path.CloseAllFigures();

            this.Region = new Region(path);
        }

        // Drag-Move für Header
        private void headerPanel_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                ReleaseCapture();
                SendMessage(this.Handle, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
        }

        // Schließen-Button
        private void btnClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        // Minimieren-Button
        private void btnMinimize_Click(object sender, EventArgs e)
        {
            this.WindowState = FormWindowState.Minimized;
        }
        private void setProgressBar()
        {
            dualBar.CurrentMaximum = 100;
            dualBar.TotalMaximum = 100;

            dualBar.CurrentValue = 0;
            dualBar.TotalValue = 0;

            dualBar.CurrentColor = Color.LimeGreen;
            dualBar.TotalColor = Color.DodgerBlue;
            dualBar.DividerColor = Color.Black;
            dualBar.TextColor = Color.Black;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // ListBox-Loks initial mit Header + Loks füllen
            RefreshLokListBox();
            InitDualBar();
            setProgressBar();
            InitStopGoButton();
            canTimer.Start();
            canTimer.Enabled = true;
            // Optional: OwnerDraw für Decoder aktivieren
            // listBoxDecoder.DrawItem += ListBoxDecoder_DrawItem;
        }

        #endregion

        #region Hilfsmethoden Basis (BitWrite, Hash, StartLoop, BridgeIP)

        // Allgemeines Bit-Setzen/Löschenprivate
        void OnGuiLog(string text)
        {
            if (InvokeRequired)
            {
                BeginInvoke(new Action(() => tbMessages.AppendText(text + "\r\n")));
            }
            else
            {
                tbMessages.AppendText(text + "\r\n");
            }
        }

        private void BitWrite(ref uint value, int bit, int bitValue)
        {
            if (bitValue != 0)
                value |= (1u << bit);   // Bit setzen
            else
                value &= ~(1u << bit);  // Bit löschen
        }

        // Hash für die CAN-UID generieren
        private void generateHash()
        {
            uint uid = 0x45009195; // CAN-UID
            uid--;
            uint highbyte = 0x4500;
            uint lowbyte = 0x9195;
            uint hash = highbyte ^ lowbyte;

            BitWrite(ref hash, 7, 0);
            BitWrite(ref hash, 8, 1);
            BitWrite(ref hash, 9, 1);

            CANFrames.SetHash((byte)(hash >> 8), (byte)hash);
        }

        // Start der CANguru-Loop (Timer)
        private void StartCANguruLoop()
        {
            looptimer.Start();
        }

        // Prüfen, ob Bridge-IP gesetzt ist
        private bool BridgeIPIsNotSet()
        {
            return string.IsNullOrEmpty(ProtocolConfig.ipString);
        }

        #endregion

        #region UDP / Paketverarbeitung

        // UDP-Start: GUI-Port öffnen, Receiver starten
        private void StartUdp()
        {
            // GUI hört auf GUI_PORT
            udpReceiver = new UdpClient(ProtocolConstants.GUI_PORT);
            udpReceiver.BeginReceive(OnUdpData, null);

            // Neues Startsignal an die Bridge (Broadcast)
            var packet = PacketBuilder.Build((byte)ProtocolConstants.FEATURE_BRIDGE, (byte)ProtocolConstants.BRIDGE_START, Array.Empty<byte>());
            UdpClientSingleton.Instance.SendBroadcast(packet);
        }

        // Zum Senden an die Bridge
        public void SendToBridge(byte[] frame)
        {
            UdpClientSingleton.Instance.Send(frame);
        }

        private void OnUdpData(IAsyncResult ar)
        {
            if (guiClosing)
                return;

            IPEndPoint remote = new IPEndPoint(IPAddress.Any, 0);

            byte[] data;

            try
            {
                data = udpReceiver.EndReceive(ar, ref remote);
            }
            catch (ObjectDisposedException)
            {
                // Socket wurde geschlossen → normal beim Beenden
                return;
            }
            catch (SocketException)
            {
                // UDP wurde abgebrochen → normal beim Beenden
                return;
            }

            // Wenn GUI gerade geschlossen wird → keine Verarbeitung mehr
            if (guiClosing)
                return;

            ProcessIncomingPacket(data, remote);

            try
            {
                udpReceiver.BeginReceive(OnUdpData, null);
            }
            catch
            {
                // Beim Schließen kann auch BeginReceive fehlschlagen → ignorieren
            }
        }

        private int UpdateAliveField(System.Windows.Forms.TextBox box, int lastValue, int newValue)
        {
            if (InvokeRequired)
            {
                return (int)Invoke(new Func<int>(() => UpdateAliveField(box, lastValue, newValue)));
            }

            box.Text = newValue.ToString();

            if (lastValue != -1 && newValue < lastValue)
                box.BackColor = Color.Red;
            else
                box.BackColor = SystemColors.Window;

            return newValue;
        }

        private async void HandleAlivePacket(IPAddress bridgeIp, byte[] payload)
        {
            firstAliveReceived = false;

            // Bridge ist online
            currentDecoderStatus = DecoderStatus.Healthy;

            // Bridge-IP übernehmen
            Log("Bridge hat sich gemeldet!");
            ProtocolConfig.BridgeIP = bridgeIp;
            ProtocolConfig.ipString = bridgeIp.ToString();
            UiInvoke(() => txtBridgeIP.Text = ProtocolConfig.ipString);

            // STOP/GO aktivieren
            UiInvoke(() => btnStopGo.Enabled = true);

            UiInvoke(() => dualBar.Visible = true);
            bool ok = await uploader.StartUploadAsync("lokomotive.cs2");
            if (!ok)
            {
                MessageBox.Show(
                    "Die Bridge ist nicht im OTA-Startmodus.\r\n" +
                    "Bitte Decoder auswählen und OTA erneut starten.",
                    "Upload nicht möglich",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Warning
                );
                return;
            }

            UiInvoke(() => dualBar.Visible = false);

            canguruStatus = enum_canguruStatus.startGleisbox;
        }

        void HandleOtaResultFromBridge(byte[] data)
        {
            if (data.Length < 4) return;

            var f = new GuiOtaResultFrame
            {
                type = data[0],
                decoderId = data[1],
                result = data[2],
                errorCode = data[3]
            };

            if (f.result == 0)
            {
                // Erfolg
                Log($"OTA erfolgreich für Decoder {f.decoderId}");
            }
            else
            {
                // Fehler
                Log($"OTA FEHLER für Decoder {f.decoderId}, Code {f.errorCode}");
            }
        }
        // ProcessIncomingPacket ersetzen, um den fehlenden Switch-Block zu beheben
        private async void ProcessIncomingPacket(byte[] data, IPEndPoint remote)
        {
            if (guiClosing)
                return;

            if (PacketParser.TryParse(data, out var featureId, out var commandId, out var payload))
            {
                switch (featureId)
                {
                    case ProtocolConstants.FEATURE_DECODER:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.FRAME_DECODER_ADDED:
                                    {
                                        byte id = payload[0];
                                        DecoderType type = (DecoderType)payload[1];
                                        byte[] mac = payload.Skip(2).Take(6).ToArray();
                                        int value0 = payload[8];
                                        int value1 = payload[9];

                                        HandleDecoderAdded(id, type, mac, value0, value1);
                                    }
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_DECODER_UPLOAD:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.DECODER_UPLOAD_START:
                                    {
                                        totalBlocks = (data[1] << 8) | data[2];
                                        UiInvoke(() => dualBar.CurrentValue = 1);
                                        UiInvoke(() => dualBar.CurrentText = $"1%");
                                    }
                                    break;

                                case ProtocolConstants.DECODER_UPLOAD_BLOCK:
                                    {
                                        int percent = payload[0];
                                        int id = payload[1];
                                        if (percent == 0)
                                        {
                                            Log("Decoder lädt Firmware...");
                                            UiInvoke(() => dualBar.Visible = true);
                                        }
                                        this.BeginInvoke(new Action(() =>
                                        {
                                            dualBar.CurrentValue = percent;
                                            dualBar.CurrentText = $"{percent}%";
                                            dualBar.Invalidate();
                                        }));
                                        if (percent == 100)
                                        {
                                            UiInvoke(() => dualBar.Visible = false);
                                            Log("Decoder-Update erfolgreich.");
                                        }
                                    }
                                    break;

                                case ProtocolConstants.DECODER_UPLOAD_END:
                                    Log("OTA erfolgreich abgeschlossen.");
                                    UiInvoke(() => dualBar.Visible = false);
                                    break;

                                case ProtocolConstants.DECODER_UPLOAD_ERROR:
                                    Log("OTA Fehler: " + data[1]);
                                    AbortOtaAndRecover();
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_SHOW_BAR:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.FRAME_GUI_OTA_CURRENT_DECODER:
                                    byte decoderIdchg = payload[0];
                                    Log("Aktueller Decoder: " + decoderIdchg);
                                    this.Invoke(new Action(() =>
                                    {
                                        dualBar.CurrentText = $"0%";
                                        dualBar.CurrentValue = 0;
                                    }));
                                    break;

                                case ProtocolConstants.FRAME_GUI_OTA_OVERALL:
                                    byte percentOverall = payload[0];
                                    if (percentOverall > 100)
                                        percentOverall = 100;
                                    if (percentOverall == 0)
                                    {
                                        UiInvoke(() => dualBar.Visible = true);
                                    }
                                    this.Invoke(new Action(() =>
                                    {
                                        if (dualBar != null)
                                        {
                                            dualBar.TotalValue = percentOverall;
                                            dualBar.TotalText = $"{percentOverall}%";
                                            dualBar?.Invalidate();
                                        }
                                    }));
                                    if (percentOverall == 100)
                                    {
                                        UiInvoke(() => dualBar.Visible = false);
                                    }
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_ALIVE:
                        {
                            switch (commandId)
                            {
                                // aliveManager.ProcessAliveMessage(receivedLine);

                                case ProtocolConstants.HANDLE_ALIVE:
                                    {
                                        // Neue Alive-Summary
                                        if (payload.Length >= 4)
                                        {
                                            int power = payload[0];
                                            int sw = payload[1];
                                            int signal = payload[2];
                                            int feedback = payload[3];

                                            aliveManager.ProcessAliveSummary(power, sw, signal, feedback);
                                        }
                                    }
                                    break;
                                case ProtocolConstants.FRAME_DECODER_ALIVE:
                                    {
                                        if (firstAliveReceived)
                                            HandleAlivePacket(remote.Address, payload);
                                        int decoderId = payload[0];
                                        CANguru.Models.DecoderType decoderType = (CANguru.Models.DecoderType)payload[1];
                                        DecoderStatus status = (DecoderStatus)payload[2];
                                        int fwMajor = payload[3];
                                        int fwMinor = payload[4];
                                        int current01 = payload[5];
                                        int current02 = payload[6];
                                        int current11 = payload[7];
                                        int current12 = payload[8];
                                        bool exists = decoderStorage.Decoders.Values.Any(d => d.AssignedId == decoderId);
                                        if (!exists)
                                            break;
                                        OnDecoderAlive(decoderId);
                                        var dec = listBoxDecoder.Items.OfType<CANguruDecoder>().FirstOrDefault(d => d.AssignedId == decoderId);

                                        if (dec == null) break;
                                        dec.IsReady = status;
                                        if (decoderType == DecoderType.Power)
                                        {
                                            int mA0 = (current01 << 8) | current02;
                                            PowerCtrl.UpdateCurrent(0, mA0);
                                            int mA1 = (current11 << 8) | current12;
                                            PowerCtrl.UpdateCurrent(1, mA1);
                                        }
                                        dec.LastSeen = DateTime.Now;
                                        //                            UpdateDecoderRow(id);
                                    }
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_CS2:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.CMD_FILE_ACK_CS2:
                                    uploader.HandleAck(data);
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_SETTING:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.SETTING_CMD_RESET_ACK:
                                    byte assignedId = payload[0];
                                    byte decoderType = payload[1];

                                    CurrentSettingsFormManager.OnResetAck(assignedId, decoderType);
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_SWITCH:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.SWITCH_CMD_GET_SETTINGS:
                                    DecoderProtocol.RaiseResponse(featureId, commandId, payload);
                                    break;
                                case ProtocolConstants.SWITCH_CMD_SET_END_ACK:
                                    activeSwitchForm?.HandleSetEndAck(featureId, commandId, payload);
                                    break;
                                case ProtocolConstants.SWITCH_CMD_SET_DIR:
                                    activeSwitchForm?.HandleSetCurrPos(featureId, commandId, payload);
                                    break;
                            }
                        }
                        return;
                    case ProtocolConstants.FEATURE_SIGNAL:
                        {
                            switch (commandId)
                            {
                                case ProtocolConstants.SIGNAL_CMD_GET_SETTINGS:
                                    DecoderProtocol.RaiseResponse(featureId, commandId, payload);
                                    break;
                                case ProtocolConstants.SIGNAL_CMD_SET_END_ACK:
                                    activeSignalForm?.HandleSetEndAck(featureId, commandId, payload);
                                    break;
                                case ProtocolConstants.SIGNAL_CMD_SET_DIR:
                                    activeSignalForm?.HandleSetCurrPos(featureId, commandId, payload);
                                    break;
                            }
                        }
                        return;
                    case ProtocolConstants.FEATURE_POWER:
                        {
                            switch (commandId)
                            {
                                // ⭐ NEU: PowerDecoder – Live-Stromwerte

                                // ⭐ NEU: PowerDecoder – Threshold bestätigt
                                case ProtocolConstants.POWER_CMD_SET_THRESHOLD:
                                    {
                                        ushort thr = (ushort)((payload[0] << 8) | payload[1]);
                                        PowerCtrl.Threshold_mA = thr;
                                        break;
                                    }

                                case ProtocolConstants.POWER_CMD_GET_THRESHOLD:
                                    {
                                        DecoderProtocol.RaiseResponse(featureId, commandId, payload);
                                        break;
                                    }

                                // ⭐ NEU: PowerDecoder – Shutdown
                                case ProtocolConstants.POWER_CMD_SHUTDOWN:
                                    {
                                        ushort mA = (ushort)((payload[0] << 8) | payload[1]);
                                        PowerCtrl.UpdateCurrent(0, mA);
                                        PowerCtrl.TriggerShutdown(0);
                                    }
                                    break;
                                // ⭐ NEU: PowerDecoder – Track Current
                                case ProtocolConstants.POWER_CHG_TRACK_CURRENT:
                                    {
                                        if (payload[0] == 0)
                                        {
                                            Log("STOPP gedrückt – Gleisspannung ausgeschaltet.");
                                            btnStopGo.SetMode(false); // => IsGoMode
                                        }
                                        else
                                        {
                                            Log("GO gedrückt – Gleisspannung liegt an.");
                                            btnStopGo.SetMode(true);// => IsSTOPPMode
                                        }
                                    }
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_FEEDBACK:
                        {
                            if (commandId == ProtocolConstants.FEEDBACK_CMD_GET_SETTINGS)
                            {
                                // 1. Maske aktualisieren
                                activeFeedbackForm?.HandleGetSettingsAck(featureId, commandId, payload);

                                // 2. DecoderProtocol informieren (für UpdateFromDecoder)
                                DecoderProtocol.RaiseResponse(featureId, commandId, payload);
                            }

                            if (commandId == ProtocolConstants.FEEDBACK_CMD_SET_SETTINGS_ACK)
                                activeFeedbackForm?.HandleSetSettingsAck(featureId, commandId, payload);

                            break;
                        }
                    case ProtocolConstants.FEATURE_FIRMWARE:
                        {
                            // FEATURE_FIRMWARE wird nur in der Kommunikation zwischen GUI und Bridge für OTA genutzt
                            // Bridge und Decoder nutzen dafür FEATURE_OTA
                            switch (commandId)
                            {
                                case ProtocolConstants.FRAME_GUI_OTA_RESULT:
                                    HandleOtaResultFromBridge(data);
                                    break;

                                case ProtocolConstants.FRAME_GUI_OTA_CURRENT_FINISHED:
                                    Log("Gruppen-Update abgeschlossen");
                                    break;
                            }
                            return;
                        }
                    case ProtocolConstants.FEATURE_OTA:
                        {
                            if (commandId == ProtocolConstants.CMD_OTA_ACK)
                            {
                                otaUploader.HandleAckPayload(payload);
                            }
                            return;
                        }
                }
            }
            // CAN-Frame (seq + payload)
            if (data.Length > ProtocolConstants.CAN_FRAME_SIZE)
            {
                uint seq = BitConverter.ToUInt32(data, 0);
                byte[] canFrame = data.Skip(4).ToArray();
                incomingFrames.Enqueue((seq, canFrame));
                return;
            }
        }



        #endregion
        #region Power-Anzeige
        /*
         *  | Aufgabe                      | Ort                            |
            | ---------------------------- | ------------------------------ |
            | Daten speichern              | **PowerDecoderModel**          |
            | Einstellungen repräsentieren | **PowerDecoderSettings**       |
            | Netzwerk senden/empfangen    | **GUI‑Code / Controller‑Code** |
            | UI aktualisieren             | **GUI‑Code**                   |
        */
        private void UpdatePowerUI()
        {
            if (guiClosing) return;
            this.BeginInvoke(new Action(() =>
            {
                labelCurrent0.Text = $"{PowerCtrl.Current0_mA} mA";
                labelCurrent1.Text = $"{PowerCtrl.Current1_mA} mA";


                panelCurrent0.BackColor =
                    PowerCtrl.Current0_mA > PowerCtrl.Threshold_mA ? Color.Red : Color.LightGreen;

                panelCurrent1.BackColor =
                    PowerCtrl.Current1_mA > PowerCtrl.Threshold_mA ? Color.Red : Color.LightGreen;
            }));

        }

        private void PowerShutdownUI(int channel)
        {
            this.BeginInvoke(new Action(() =>
            {
                if (channel == 0)
                    panelCurrent0.BackColor = Color.Red;
                else
                    panelCurrent1.BackColor = Color.Red;
            }));
        }

        private void buttonSettings_Click(object sender, EventArgs e)
        {
            if (listBoxDecoder.SelectedItem is not CANguruDecoder decoder)
            {
                MessageBox.Show("Bitte einen Decoder auswählen.");
                return;
            }

            switch (decoder.Type)
            {
                case DecoderType.Switch:
                    byte switchassignedId = (byte)decoder.AssignedId;

                    activeSwitchForm = new SwitchDecoderSettingsForm(switchassignedId);
                    activeSwitchForm.ShowDialog(this);
                    activeSwitchForm = null; // nach dem Schließen aufräumen
                    return;
                case DecoderType.Signal:
                    byte signalassignedId = (byte)decoder.AssignedId;

                    activeSignalForm = new SignalDecoderSettingsForm(signalassignedId);
                    activeSignalForm.ShowDialog(this);
                    activeSignalForm = null; // nach dem Schließen aufräumen
                    return;
                case DecoderType.Feedback:
                    {
                        byte feedbackassignedId = (byte)decoder.AssignedId;

                        activeFeedbackForm = new FeedbackDecoderSettingsForm(feedbackassignedId);
                        activeFeedbackForm.ShowDialog(this);
                        activeFeedbackForm = null;
                        return;
                    }
                    return;
                case DecoderType.Power:
                    byte powerAssignedId = (byte)decoder.AssignedId;
                    new PowerDecoderSettingsForm(powerAssignedId).ShowDialog(this);
                    return; // weiter zu Power-Einstellungen
                default:
                    MessageBox.Show("Diesen Decoder gibt es nicht.");
                    return;
            }
        }

        #endregion Power-Anzeige

        #region Over The Air
        struct GuiOtaStartWifiFrame
        {
            public byte type;
            public byte decoderId;
            public byte fwMajor;
            public byte fwMinor;
        }
        struct GuiOtaResultFrame
        {
            public byte type;
            public byte decoderId;
            public byte result;    // 0 = success, 1 = error
            public byte errorCode; // bei success = 0
        }
        private void StopAllGuiTimers()
        {
            aliveTimer?.Stop();
            canTimer?.Stop();
            PINGtimer?.Stop();
        }
        private void StartAllGuiTimers()
        {
            aliveTimer?.Start();
            canTimer?.Start();
            PINGtimer?.Start();
        }

        private void UpdateProgressSafe(int current, int total)
        {
            if (InvokeRequired)
            {
                BeginInvoke(new Action(() => UpdateProgressSafe(current, total)));
                return;
            }

            int percent = (int)(current * 100.0 / total);
            Log($"Progress: {percent}% ({current}/{total})");
            if (percent > lastPercent)
                lastPercent++;

            this.BeginInvoke(new Action(() =>
            {
                dualBar.CurrentValue = lastPercent;
                dualBar.CurrentText = $"{lastPercent}%";
                dualBar.Invalidate();
            }));
            //    dualBar.CurrentValue = percent;
        }

        private async void btnOtaUpdate_Click(object sender, EventArgs e)
        {
            byte id = 0;

            // Einzeldecoder auswählen
            if (radioSingle.Checked)
            {
                if (listBoxDecoder.SelectedItem is not CANguruDecoder decoder)
                {
                    MessageBox.Show("Bitte einen Decoder auswählen.");
                    return;
                }
                id = (byte)decoder.AssignedId;
            }

            Log("Over The Air-Prozess gestartet.");

            // Firmware auswählen
            using var ofd = new OpenFileDialog();
            ofd.Filter = "Firmware (*.bin)|*.bin";

            if (ofd.ShowDialog() != DialogResult.OK)
                return;

            byte[] firmware = File.ReadAllBytes(ofd.FileName);
            Log($"Firmware-Datei wird geladen ({firmware.Length} Bytes).");

            // UI vorbereiten
            dualBar.Visible = true;
            dualBar.SingleBarMode = true;
            dualBar.CurrentValue = 0;
            lastPercent = 0;
            isFirmwareUploadActive = true;
            StopAllGuiTimers();

            _uploadCts = new CancellationTokenSource();
            bool ok = false;

            try
            {
                // ⭐ NEU: OTA-Uploader benutzen
                ok = await otaUploader.UploadFirmwareAsync(firmware, _uploadCts.Token);
            }
            catch (OperationCanceledException)
            {
                Log("Firmware-Upload abgebrochen.");
            }
            catch (Exception ex)
            {
                Log($"Firmware-Upload Fehler: {ex.Message}");
            }

            isFirmwareUploadActive = false;
            StartAllGuiTimers();

            if (!ok)
            {
                Log("Firmware-Upload fehlgeschlagen.");
                dualBar.Visible = false;
                dualBar.SingleBarMode = false;
                return;
            }

            Log("Firmware erfolgreich an Bridge übertragen.");
            dualBar.Visible = true;

            // ⭐ OTA-Start an Bridge senden

            // Einzeldecoder
            if (radioSingle.Checked)
            {
                dualBar.SingleBarMode = true;

                var pload = new byte[] { id, 0x00 };
                var pkt = PacketBuilder.Build(
                    (byte)ProtocolConstants.FEATURE_OTA,
                    (byte)ProtocolConstants.OTA_SINGLE_DECODER,
                    pload);

                UdpClientSingleton.Instance.Send(pkt);
                return;
            }

            // Gruppendecoder
            byte? type = null;
            if (radioType1.Checked) type = ProtocolConstants.DEVTYPE_SWITCH;
            else if (radioType2.Checked) type = ProtocolConstants.DEVTYPE_SIGNAL;
            else if (radioType3.Checked) type = ProtocolConstants.DEVTYPE_POWER;
            else if (radioType4.Checked) type = ProtocolConstants.DEVTYPE_RM;

            if (type == null)
            {
                MessageBox.Show("Bitte einen OTA-Modus auswählen.");
                return;
            }

            dualBar.SingleBarMode = false;

            var payload = new byte[] { id, type.Value };
            var packet = PacketBuilder.Build(
                (byte)ProtocolConstants.FEATURE_OTA,
                (byte)ProtocolConstants.OTA_GROUP_DECODERS,
                payload);

            UdpClientSingleton.Instance.Send(packet);
        }

        private void AbortOtaAndRecover()
        {
            UiInvoke(() =>
            {
                MessageBox.Show(
                    "Beim Firmware-Update ist ein Fehler aufgetreten.\n\n" +
                    "Das gesamte System (GUI, Bridge und Decoder) wird jetzt neu gestartet.",
                    "OTA-Fehler",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Warning
                );
            });

            ResetGuiToInitialState();
        }

        private void ResetGuiToInitialState()
        {
            Log("GUI wird in den Anfangszustand versetzt...");

            // 1. UI zurücksetzen
            UiInvoke(() =>
            {
                listBoxDecoder.Items.Clear();
                dualBar.Visible = false;
                dualBar.SingleBarMode = false;

                dualBar.CurrentValue = 0;
                dualBar.TotalValue = 0;
                dualBar.CurrentText = "";
                dualBar.TotalText = "";
                dualBar.Invalidate();
            });

            // 2. interne Objekte neu erzeugen
            CANFrames = new CANFrames();
            generateHash();
            mfxParser = new MfxParser(lokRegistry, CANFrames);
            decoderController = new DecoderList();
            decoderStorage = new DecoderStorage();

            // Parser-Callbacks neu setzen
            mfxParser.LogMessage += OnParserLog;
            mfxParser.incnextLocid += OnincnextLocid;
            mfxParser.LokCompleted += OnLokCompleted;
            mfxParser.SendFrameRequested += OnSendFrameRequested;

            // 3. UDP neu starten
            try
            {
                udpReceiver?.Close();
            }
            catch { }

            StartUdp();

            // 4. Alive zurücksetzen
            currentDecoderStatus = DecoderStatus.Lost;

            // 5. CANguru-Status zurücksetzen
            canguruStatus = enum_canguruStatus.idle;

            Log("GUI-Reset abgeschlossen.");
        }

        #endregion over the air

        #region lokomotive.cs2 -Upload
        private void InitUploader()
        {
            uploader = new Cs2FileUploader();

            uploader.ProgressChanged += (offset, total) =>
            {
                UiInvoke(() =>
                    { // Wert aktualisieren
                        dualBar.SingleBarMode = true;
                        dualBar.CurrentValue = offset * 100 / total;
                        dualBar.CurrentText = $"{(offset * 100 / total)}%";
                        dualBar.Invalidate();
                    });
            };

            uploader.UploadCompleted += () =>
            {
                Log("Upload lokomotive.cs2 abgeschlossen!");
                dualBar.SingleBarMode = false;
            };

            uploader.UploadError += (msg) =>
            {
                MessageBox.Show("Fehler beim Upload lokomotive.cs2: " + msg);
            };
        }
        #endregion

        #region Alive-Überwachung (canTimer/aliveTimer + Statusanzeige)

        // Alive-Timer (100 ms)
        private void canTimer_Tick(object sender, EventArgs e)
        {
            if (BridgeIPIsNotSet())
                return;

            // CAN-Frames sortieren + anzeigen
            ProcessPendingCanFrames();

            // Outgoing CAN senden
            ProcessOutgoingCanFrames();
        }

        public void OnDecoderAlive(int decoderId)
        {
            if (decoderStorage.Decoders.TryGetValue((uint)decoderId, out var decoder))
            {
                decoder.LastSeen = DateTime.Now;
            }
        }

        // Alive-Statusanzeige
        private void UpdateUI()
        {
            var decoderSnapshot = decoderStorage.Decoders.Values.ToList();

            foreach (var decoder in decoderSnapshot)
            {
                switch (decoder.IsReady)
                {
                    case DecoderStatus.Healthy:
                        lblBRIDGEStatus.Text = "ONLINE";
                        lblBRIDGEStatus.BackColor = Color.LimeGreen;
                        break;

                    case DecoderStatus.Weak:
                        lblBRIDGEStatus.Text = "WEAK";
                        lblBRIDGEStatus.BackColor = Color.Gold;
                        break;

                    case DecoderStatus.Lost:
                        lblBRIDGEStatus.Text = "OFFLINE";
                        lblBRIDGEStatus.BackColor = Color.Red;
                        break;
                }
            }
        }

        // GUI sendet Alive an Bridge
        private void aliveTimer_Tick(object sender, EventArgs e)
        {
            foreach (var decoder in decoderStorage.Decoders.Values)
            {
                var delta = DateTime.Now - decoder.LastSeen;

                if (delta.TotalSeconds < 2)
                {
                    decoder.IsReady = DecoderStatus.Healthy;
                }
                else if (delta.TotalSeconds < 5)
                {
                    decoder.IsReady = DecoderStatus.Weak;
                }
                else
                {
                    decoder.IsReady = DecoderStatus.Lost;
                }
            }
            UpdateUI();
        }


        private void SendCANFrame2Bridge(byte[] canFrame)
        {
            // canFrame hat ein Zeichen mehr als CAN_FRAME_SIZE: "G"

            // Nur die ersten 13 Bytes senden
            byte[] frameToSend = new byte[ProtocolConstants.CAN_FRAME_SIZE];
            Array.Copy(canFrame, frameToSend, ProtocolConstants.CAN_FRAME_SIZE);

            UdpClientSingleton.Instance.Send(frameToSend);
            DisplayCanFrame(canFrame);
        }

        // Eingehende CAN-Frames aus Queue holen, sortieren und verarbeiten
        private void ProcessPendingCanFrames()
        {
            if (incomingFrames.IsEmpty)
                return;

            List<(uint seq, byte[] data)> list = new();
            while (incomingFrames.TryDequeue(out var item))
                list.Add(item);

            list.Sort((a, b) => a.seq.CompareTo(b.seq));

            foreach (var frame in list)
            {
                DisplayCanFrame(frame.data);
                mfxParser.ProcessCanFrame(frame.data);
            }
        }

        // Ausgehende CAN-Frames aus Queue senden
        private void ProcessOutgoingCanFrames()
        {
            while (outgoingFrames.TryDequeue(out var frame))
                UdpClientSingleton.Instance.Send(frame);
        }

        // CAN-Frame als Text darstellen
        private void DisplayCanFrame(byte[] data)
        {
            string strFrame;
            byte currCMD = data[0x01];
            byte subCmd = data[0x09];
            byte dlc = data[0x04];
            char source = (char)data[0x0D];

            strFrame = System.String.Format("{0}-{1:X2}({2:X2}){3:X2}{4:X2}", source, data[0], currCMD, data[2], data[3]);

            if ((currCMD & 0x01) == 0x01)
                strFrame += " R ";
            else
                strFrame += "   ";

            strFrame += System.String.Format("[{0}]", dlc);

            for (byte i = 5; i < 5 + dlc; i++)
                strFrame += System.String.Format(" {0:X2}", data[i]);

            if (dlc < 8)
            {
                strFrame += System.String.Format("({0:X2}", data[(byte)(5 + dlc)]);
                for (byte i = (byte)(6 + dlc); i < 13; i++)
                    strFrame += System.String.Format(" {0:X2}", data[i]);
                strFrame += ") ";
            }
            else
            {
                strFrame += "  ";
            }

            strFrame += "  ";

            byte[] bytes = new byte[8];
            for (int i = 5; i < 13; i++)
                bytes[i - 5] = data[i];

            char[] chars = Encoding.UTF8.GetChars(bytes);
            for (int i = 0; i < 8; i++)
            {
                if (chars[i] < ' ' || chars[i] > 'z')
                    strFrame += ".";
                else
                    strFrame += chars[i];
            }

            switch (currCMD)
            {
                case 0x00:
                    switch (subCmd)
                    {
                        case 0x00: strFrame += "Sys STOPP"; break;
                        case 0x01: strFrame += "Sys GO"; break;
                        case 0x02: strFrame += "Sys Halt"; break;
                        case 0x03: strFrame += "Lok Halt"; break;
                        case 0x04: strFrame += "Lok Stop"; break;
                        case 0x05: strFrame += "Lok Daten"; break;
                        case 0x06: strFrame += "Schaltzeit"; break;
                        case 0x07: strFrame += "Fast Read"; break;
                        case 0x08: strFrame += "Glsproto"; break;
                        case 0x09: strFrame += "Anm.zähler"; break;
                        case 0x0A: strFrame += "Überlast"; break;
                        case 0x0B: strFrame += "Status"; break;
                        case 0x0C: strFrame += "Kennung"; break;
                        case 0x30: strFrame += "Mfx Seek"; break;
                        case 0x80: strFrame += "Reset"; break;
                    }
                    break;
                case 0x02: strFrame += "Discovery"; break;
                case 0x04: strFrame += "MFX Bind"; break;
                case 0x06: strFrame += "MFX Verify"; break;
                case 0x08: strFrame += "Lok Speed"; break;
                case 0x0A: strFrame += "Richtung"; break;
                case 0x0C: strFrame += "Funktion"; break;
                case 0x0E: strFrame += "Read Config"; break;
                case 0x10: strFrame += "Write Config"; break;
                case 0x16: strFrame += "Zub Schalten"; break;
                case 0x18: strFrame += "Zub Konfig"; break;
                case 0x20: strFrame += "S88 Feedback"; break;
                case 0x22: strFrame += "S88 Event"; break;
                case 0x24: strFrame += "SX1 Event"; break;
                case 0x30: strFrame += "Ping"; break;
                case 0x32: strFrame += "Update"; break;
                case 0x34: strFrame += "Config Data"; break;
                case 0x36: strFrame += "Magic"; break;
                case 0x38: strFrame += "Bootloader"; break;
                case 0x3A: strFrame += "Send Konfi"; break;
                case 0x40: strFrame += "Data Query"; break;
                case 0x42: strFrame += "Data Stream"; break;
                case 0x44: strFrame += "6021 adapter"; break;
                default:
                    if (currCMD % 2 == 0)
                        strFrame += "unbekannt";
                    else
                        strFrame += System.String.Format("RE: {0:X2}", currCMD);

                    const int maxString = 60;
                    if (strFrame.Length > maxString)
                        strFrame = strFrame.Substring(0, maxString);
                    break;
            }

            UiInvoke(() =>
                {
                    listBoxCAN.AppendText(strFrame);
                    listBoxCAN.AppendText(Environment.NewLine);
                    if (currCMD < 0x08 && currCMD > 0x00)
                    {
                        listBoxMFX.AppendText(strFrame);
                        listBoxMFX.AppendText(Environment.NewLine);
                    }
                });
        }

        #endregion

        #region Decoder-Handling (Liste + Zeichnen)

        private void HandleDecoderAdded(byte id, DecoderType type, byte[] mac, int value_0, int value_1)
        {
            CANguruDecoder decoder;

            switch (type)
            {
                case DecoderType.Power:
                    decoder = new PowerDecoder();
                    break;

                case DecoderType.Switch:
                    decoder = new SwitchDecoder();
                    if (decoder is SwitchDecoder _switch)
                    {
                        Task.Run(async () =>
                        {
                            await Task.Delay(300); // Decoder Zeit geben
                            _switch.UpdateFromDecoder();
                        });
                    }
                    break;

                case DecoderType.Signal:
                    decoder = new SignalDecoder();
                    if (decoder is SignalDecoder _signal)
                    {
                        Task.Run(async () =>
                        {
                            await Task.Delay(300); // Decoder Zeit geben
                            _signal.UpdateFromDecoder();
                        });
                    }
                    break;

                case DecoderType.Feedback:
                    decoder = new FeedbackDecoder();
                    if (decoder is FeedbackDecoder _feedback)
                    {
                        Task.Run(async () =>
                        {
                            await Task.Delay(300); // Decoder Zeit geben
                            _feedback.UpdateFromDecoder();
                        });
                    }
                    break;

                default:
                    decoder = new CANguruDecoder();
                    break;
            }

            decoder.AssignedId = id;
            decoder.Type = type;
            decoder.Mac = mac;
            decoder.value0 = value_0;
            decoder.value1 = value_1;
            decoder.LastSeen = DateTime.Now;

            Log($"Neuer Decoder hinzugefügt: ID={decoder.AssignedId}, Type={decoder.Type}");

            decoderStorage.AddOrUpdate(decoder);
            RefreshDecoderListBox();
        }

        // Decoder-ListBox neu aufbauen (Header + Decoder)
        private void RefreshDecoderListBox()
        {
            if (InvokeRequired)
            {
                BeginInvoke(new Action(RefreshDecoderListBox));
                return;
            }

            listBoxDecoder.Items.Clear();
            listBoxDecoder.Items.Add("HEADER");
            listBoxDecoder.Items.Add("SEPARATOR");
            foreach (var decoder in decoderStorage.Decoders.Values.OrderBy(l => l.Name))
                listBoxDecoder.Items.Add(decoder);
        }

        // OwnerDraw für Decoder-ListBox
        private void ListBoxDecoder_DrawItem(object sender, DrawItemEventArgs e)
        {
            if (e.Index < 0)
                return;

            var lb = (ListBox)sender;
            var item = lb.Items[e.Index];

            e.DrawBackground();

            // Header-Zeile
            if (item is string s && s == "HEADER")
            {
                using var bg = new SolidBrush(Color.LightGray);
                e.Graphics.FillRectangle(bg, e.Bounds);

                string header = "Name                      MAC                  UID";

                using var textBrush = new SolidBrush(Color.Black);
                e.Graphics.DrawString(header, lb.Font, textBrush, e.Bounds.Left + 2, e.Bounds.Top + 1);
                return;
            }

            // Separator-Zeile
            if (item is string sep && sep == "SEPARATOR")
            {
                using var bg = new SolidBrush(Color.Gray);
                e.Graphics.FillRectangle(bg, e.Bounds);

                string line = "---------------------------------------------------";

                using var textBrush = new SolidBrush(Color.White);
                e.Graphics.DrawString(line, lb.Font, textBrush, e.Bounds.Left + 2, e.Bounds.Top + 1);
                return;
            }

            // Normale Decoder-Zeile
            if (item is CANguruDecoder decoder)
            {
                string macStr = $"{decoder.Mac[0]:X2}:{decoder.Mac[1]:X2}:{decoder.Mac[2]:X2}:{decoder.Mac[3]:X2}:{decoder.Mac[4]:X2}:{decoder.Mac[5]:X2}";
                int hexadr = 0xd715 + decoder.AssignedId;
                string adr = $"{hexadr:X4}".PadRight(8);

                string name = decoder.Type switch
                {
                    DecoderType.Switch => "Weichendecoder",
                    DecoderType.Signal => "Signaldecoder",
                    DecoderType.Power => "Stromwaechter",
                    DecoderType.Feedback => "Gleisbesetztmelder",
                    _ => "Decoder"
                };

                string line = $"{name.PadRight(25)} {macStr.PadRight(20)} {adr}";

                // 1. Hintergrundfarbe bestimmen
                Color back;
                if ((e.State & DrawItemState.Selected) == DrawItemState.Selected)
                {
                    // Benutzer hat die Zeile angeklickt → Highlight-Farbe
                    back = Color.LightBlue;   // oder SystemColors.Highlight
                }
                else
                {
                    // Normaler Zustand → DecoderStatus-Farbe
                    switch (decoder.IsReady)
                    {
                        case DecoderStatus.Weak:
                            back = Color.LightYellow;
                            break;
                        case DecoderStatus.Lost:
                            back = Color.LightCoral;
                            break;
                        default:
                            back = Color.White;
                            break;
                    }
                }

                using var backBrush = new SolidBrush(back);
                using var textBrush = new SolidBrush(e.ForeColor);

                // 2. Hintergrund zeichnen
                e.Graphics.FillRectangle(backBrush, e.Bounds);

                // 3. Text zeichnen
                e.Graphics.DrawString(line, lb.Font, textBrush, e.Bounds.Left + 2, e.Bounds.Top + 1);

                // 4. Fokusrechteck (optional)
                e.DrawFocusRectangle(); return;
            }

            // Fallback
            e.Graphics.DrawString(item.ToString(), lb.Font, Brushes.Black, e.Bounds.Left + 2, e.Bounds.Top + 1);
        }

        #endregion

        #region Lok-Handling (Liste, Zeichnen, Export, Manuelle Loks)

        private void UpdateNextMfxSID()
        {
            int maxSid = 0;

            foreach (var item in listBoxLoks.Items)
            {
                if (item is CANguruLok lok)
                {
                    if (!lok.IsManual) // echte mfx-Lok
                    {
                        if (lok.MfxSID > maxSid)
                            maxSid = lok.MfxSID;
                    }
                }
            }

            int nextSid = maxSid + 1;

            // UI-thread-sicher setzen
            if (UpDnnextLokAddress.InvokeRequired)
            {
                UpDnnextLokAddress.Invoke(new Action(() =>
                {
                    UpDnnextLokAddress.Value = nextSid;
                }));
            }
            else
            {
                UpDnnextLokAddress.Value = nextSid;
            }
        }

        private void RefreshLokListBox()
        {
            listBoxLoks.Items.Clear();

            listBoxLoks.Items.Add("HEADER");
            listBoxLoks.Items.Add("SEPARATOR");

            foreach (var lok in lokstorage.Loks.Values.OrderBy(l => l.Name))
                listBoxLoks.Items.Add(lok);
        }

        // OwnerDraw für Lok-ListBox
        private void listBoxLoks_DrawItem(object sender, DrawItemEventArgs e)
        {
            if (e.Index < 0)
                return;

            var lb = (ListBox)sender;
            var item = lb.Items[e.Index];

            e.DrawBackground();

            // Header-Zeile
            if (item is string s && s == "HEADER")
            {
                using var bg = new SolidBrush(Color.LightGray);
                e.Graphics.FillRectangle(bg, e.Bounds);

                string header = "Name                 Adr  Typ    UID";

                using var textBrush = new SolidBrush(Color.Black);
                e.Graphics.DrawString(header, lb.Font, textBrush, e.Bounds.Left + 2, e.Bounds.Top + 1);
                return;
            }

            // Separator-Zeile
            if (item is string sep && sep == "SEPARATOR")
            {
                using var bg = new SolidBrush(Color.Gray);
                e.Graphics.FillRectangle(bg, e.Bounds);

                string line = "-------------------------------------------";

                using var textBrush = new SolidBrush(Color.White);
                e.Graphics.DrawString(line, lb.Font, textBrush, e.Bounds.Left + 2, e.Bounds.Top + 1);
                return;
            }

            // Normale Lok-Zeile
            if (item is CANguruLok lok)
            {
                string name = lok.Name.PadRight(20);
                string adr = lok.MfxSID.ToString().PadRight(4);
                string typ = (lok.IsManual ? lok.DecoderType : "mfx").PadRight(6);
                string uid = lok.UID.ToString("X8").PadRight(10);

                string line = $"{name} {adr} {typ} {uid}";

                using var textBrush = new SolidBrush(e.ForeColor);
                e.Graphics.DrawString(line, lb.Font, textBrush, e.Bounds.Left + 2, e.Bounds.Top + 1);
                return;
            }

            // Fallback
            e.Graphics.DrawString(item.ToString(), lb.Font, Brushes.Black, e.Bounds.Left + 2, e.Bounds.Top + 1);
        }

        // Lok-Liste (nur Loks, ohne Header) aktualisieren
        private void UpdateLokList()
        {
            listBoxLoks.Items.Clear();

            foreach (var lok in lokstorage.Loks.Values.OrderBy(l => l.Name))
                listBoxLoks.Items.Add(lok);
            UpdateNextMfxSID();
        }

        // Callback: Lok vollständig eingelesen
        private void OnLokCompleted(CANguruLok lok)
        {
            lokstorage.AddOrUpdate(lok);
            UpdateLokList();
        }

        // Lok löschen
        private void btnDelete_Click(object sender, EventArgs e)
        {
            if (listBoxLoks.SelectedItem is CANguruLok lok)
            {
                lokstorage.Remove(lok.UID);
                UpdateLokList();
            }
        }

        // CS2-Lokliste exportieren und zur Bridge übertragen
        private async void btnLokListe_Click(object sender, EventArgs e)
        {
            var exporter = new Cs2LokFileExporter
            {
                SessionId = CANFrames.newSubscriptionCounter,
                VersionMinor = 3
            };

            string tempFile = "lokomotive.cs2";
            exporter.Export(tempFile, lokstorage.Loks.Values);

            await exporter.UploadLokFileToBridge(ProtocolConfig.ipString, tempFile);

            Log("Lokliste exportiert und zur Bridge übertragen.");
        }

        // Manuelle Lok hinzufügen
        private void btnAddManualLok_Click(object sender, EventArgs e)
        {
            using var dlg = new ManualLokDialog();

            if (dlg.ShowDialog() == DialogResult.OK)
            {
                lokstorage.AddOrUpdate(dlg.ResultLok);
                RefreshLokListBox();
            }
        }

        private void btnShowLokList_Click(object sender, EventArgs e)
        {
            Log("Lok-Liste anzeigen...");
            lokListForm.RefreshLokList(lokstorage.Loks.Values);
            lokListForm.Show();
            lokListForm.BringToFront();
        }

        #endregion

        #region CANguru-Steuerloop (Gleisbox-Start, PING, LoopTimer)

        // Gleisbox-Startprozedur
        private void proc_startGleisbox()
        {
            if (BridgeIPIsNotSet())
                return;

            SendCANFrame2Bridge(CANFrames.magicStart0);
            SendCANFrame2Bridge(CANFrames.magicStart1);
            SendCANFrame2Bridge(CANFrames.CAN_PING);
            // Spannung wird von WDP eingeschaltet
        }

        // PING-Timer
        private void PINGtimer_Tick(object sender, EventArgs e)
        {
            if (BridgeIPIsNotSet())
                return;

            SendCANFrame2Bridge(CANFrames.CAN_PING);
        }

        // Haupt-Loop für CANguru-Steuerlogik
        private void looptimer_Tick(object sender, EventArgs e)
        {
            if (BridgeIPIsNotSet())
                return;

            switch (canguruStatus)
            {
                case enum_canguruStatus.idle:
                    // nichts zu tun
                    break;

                case enum_canguruStatus.startGleisbox:
                    proc_startGleisbox();
                    canguruStatus = enum_canguruStatus.wait4LokListe;
                    break;

                case enum_canguruStatus.wait4LokListe:
                    // hier könnte später  andere Logik folgen
                    canguruStatus = enum_canguruStatus.systemIsRunning;

                    break;
                case enum_canguruStatus.systemIsRunning:
                    looptimer.Stop();
                    break;
            }
        }

        #endregion

        #region UI-Eventhandler (Buttons, UpDowns)

        // Verbinden mit Bridge
        private void btnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                btnConnect.Enabled = false;

                StartUdp();

                Log(string.Empty);
                Log("Verbinde mit CANguru-Bridge ...");
            }
            catch (Exception ex)
            {
                Log("Fehler: " + ex.Message);
            }
        }

        private void close_GUI()
        {
            try
            {
                guiClosing = true;
                Log(".");
                Log("Programm wird beendet...");
                SendCANFrame2Bridge(CANFrames.SysSTOPP);

                lblBRIDGEStatus.Text = "Offline";
                lblBRIDGEStatus.ForeColor = Color.Red;

                // 2 Sekunden warten
                System.Threading.Thread.Sleep(2000);

                Properties.Settings.Default.newSubscriptionCounter = (byte)UpDnnewSubscriptionCounter.Value;
                Properties.Settings.Default.nextLokAddress = (byte)UpDnnextLokAddress.Value;
                Properties.Settings.Default.Save();
                // Reset an Bridge senden
                var packet = PacketBuilder.Build((byte)ProtocolConstants.FEATURE_DECODER, (byte)ProtocolConstants.CMD_DECODER_RESET, Array.Empty<byte>());
                UdpClientSingleton.Instance.Send(packet);
            }
            catch
            {
                // Ignorieren, beim Beenden nicht kritisch
            }

            Application.Exit();
        }

        // Programm beenden
        private void btnExit_Click(object sender, EventArgs e)
        {
            close_GUI();
        }

        // CAN-Log in Zwischenablage kopieren
        private void btnCopy_Click(object sender, EventArgs e)
        {
            Clipboard.SetText(listBoxCAN.Text);
        }

        // UpDown: neuer Subscription-Counter
        private void UpDnnewSubscriptionCounter_ValueChanged(object sender, EventArgs e)
        {
            CANFrames.newSubscriptionCounter = (byte)UpDnnewSubscriptionCounter.Value;
        }

        // UpDown: nächste Lokadresse
        private void UpDnnextLokAddress_ValueChanged(object sender, EventArgs e)
        {
            CANFrames.nextLokAddress = (byte)UpDnnextLokAddress.Value;
        }
        private void ApplyNextMfxSIDToUI()
        {
            if (lokstorage == null)
                return;

            int next = lokstorage.NextMfxSID;

            // UI-thread-sicher
            if (UpDnnextLokAddress.InvokeRequired)
            {
                UpDnnextLokAddress.Invoke(new Action(() =>
                {
                    UpDnnextLokAddress.Value = next;
                }));
            }
            else
            {
                UpDnnextLokAddress.Value = next;
            }
        }

        // CANguru-Loks anstoßen (MFX-Scan)
        private void btnCANguruLoks_Click_1(object sender, EventArgs e)
        {
            listBoxMFX.Clear();
            STOPP_Timer.Enabled = true;
            mfxParser.handleCANguruLoks(CANFrames.newSubscriptionCounter);
        }

        // STOPP-Timer: System GO nach kurzer Zeit
        private void STOPP_Timer_Tick(object sender, EventArgs e)
        {
            SendCANFrame2Bridge(CANFrames.SysGO);
            STOPP_Timer.Enabled = false;
        }

        #endregion

        #region Parser-Callbacks und Logging

        // Parser: nächste Lok-ID
        private void OnincnextLocid(byte val)
        {
            CANFrames.nextLokAddress++;
            UpDnnextLokAddress.Value = CANFrames.nextLokAddress;
        }

        // Parser: Log-Meldung
        public void OnParserLog(string msg)
        {
            Log(msg);
        }

        // Parser: CAN-Frame soll gesendet werden
        private void OnSendFrameRequested(byte[] data)
        {
            if (BridgeIPIsNotSet())
                return;

            SendCANFrame2Bridge(data);
        }

        // Text ins Log-Fenster schreiben
        public void Log(string text)
        {
            Invoke(new Action(() =>
            {
                tbMessages.AppendText(text + "\r\n");
            }));
        }

        #endregion

        #region Utility-Methoden (Thread-Sicherheit, Senden)

        // Thread-sicherer UI-Aufruf
        private void UiInvoke(Action action)
        {
            if (InvokeRequired)
                BeginInvoke(action);
            else
                action();
        }

        #endregion
        private void BtnStopGo_StopClicked(object sender, EventArgs e)
        {
            Log("Stopp gedrückt");
            SendCANFrame2Bridge(CANFrames.SysSTOPP);
            //    btnStopGo.PerformClick();
        }

        private void BtnStopGo_GoClicked(object sender, EventArgs e)
        {
            Log("GO gedrückt");
            SendCANFrame2Bridge(CANFrames.SysGO);
            //   btnStopGo.PerformClick();
        }

        private async void showDecoderList_Click(object sender, EventArgs e)
        {
            Log("Decoder-Liste anzeigen...");

            // 1. Live-Werte holen
            foreach (var dec in decoderStorage.Decoders.Values)
            {
                // 2. Decoder Zeit geben, sich zu initialisieren
                await Task.Delay(150);
                dec.UpdateFromDecoder();
            }

            // 3. ListView neu aufbauen
            decoderListForm.LoadDecoderList();

            // 4. Anzeigen
            decoderListForm.Show();
        }
    }
}
