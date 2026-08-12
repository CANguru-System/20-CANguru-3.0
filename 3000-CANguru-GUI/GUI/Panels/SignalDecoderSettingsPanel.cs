using CANguru.GUI.Forms;
using CANguru.GUI.DecoderSettings;
using System.ComponentModel;
using System.Diagnostics;

namespace CANguru.GUI.Panels
{
    public partial class SignalDecoderSettingsPanel : DecoderSettingsPanelBase
    {
        private SignalConfigService cfg;

        private bool IsInDesigner =>
            LicenseManager.UsageMode == LicenseUsageMode.Designtime ||
            Process.GetCurrentProcess().ProcessName.Contains("devenv");

        private void BuildRuntimeUI()
        {
            if (IsInDesigner)
                return;

            // hier ALLES rein, was dynamisch ist
            cmbRightLeft.Items.Clear();
            cmbRightLeft.Items.Add("Nullpunkt (0)");
            cmbRightLeft.Items.Add("Endpunkt (1)");

            cmbStepDirection.Items.Clear();
            cmbStepDirection.Items.Add("Vorwärts (0)");
            cmbStepDirection.Items.Add("Rückwärts (1)");

            pictureBox.SizeMode = PictureBoxSizeMode.Zoom;

            // Protokoll-Konstanten
            numDelay.Minimum = ProtocolConstants.step_delay_min;
            numDelay.Maximum = ProtocolConstants.step_delay_max;
            numDelay.Increment = ProtocolConstants.step_delay_step;

            numStepsToEnd.Minimum = ProtocolConstants.stepsToEnd_min;
            numStepsToEnd.Maximum = ProtocolConstants.stepsToEnd_max;
            numStepsToEnd.Increment = ProtocolConstants.stepsToEnd_step;

            btnSlowMove2Nullpkt.AutoSize = true;
        }
        private void InitStaticUI()
        {
            lblAddress.Text = "Weichenadresse:";
            lblDelay.Text = "Verzögerung (ms):";
            lblSteps.Text = "Gesamtschritte:";
            lblRightLeft.Text = "Momentane Position:";
            lblStepDir.Text = "Laufrichtung:";

            lblAddress.AutoSize = true;
            lblDelay.AutoSize = true;
            lblSteps.AutoSize = true;
            lblRightLeft.AutoSize = true;
            lblStepDir.AutoSize = true;

            btnSlowMove2Nullpkt.Text = "Langsam zur Nullposition";
            btnSetZero.Text = "Nullpunkt festlegen";
            btnSlowMove2End.Text = "Langsam zur Endposition";
            btnSetEnd.Text = "Endpunkt festlegen";
            btnTest.Text = "Testfahrt";

            btnSlowMove2Nullpkt.AutoSize = true;
            btnSetZero.AutoSize = true;
            btnSlowMove2End.AutoSize = true;
            btnSetEnd.AutoSize = true;
            btnTest.AutoSize = true;
        }

        // Designer-Konstruktor
        public SignalDecoderSettingsPanel()
        {
            InitializeComponent();

            // statische UI (Texte etc.) DARF im Designer laufen
            InitStaticUI();

            // Layout NICHT hier – das macht die Designer-Datei

            if (!IsInDesigner)
                BuildRuntimeUI();   // nur Werte, keine Positionen
        }

        // Runtime-Konstruktor
        public SignalDecoderSettingsPanel(byte assignedId) : this()
        {
            if (IsInDesigner)
                return;

            if (!IsInDesigner)
            {
                numDelay.Minimum = ProtocolConstants.step_delay_min;
                numDelay.Maximum = ProtocolConstants.step_delay_max;
                numDelay.Increment = ProtocolConstants.step_delay_step;

                numStepsToEnd.Minimum = ProtocolConstants.stepsToEnd_min;
                numStepsToEnd.Maximum = ProtocolConstants.stepsToEnd_max;
                numStepsToEnd.Increment = ProtocolConstants.stepsToEnd_step;
            }

            cfg = new SignalConfigService(assignedId);
            WireEvents();
        }

        private void WireEvents()
        {
            if (IsInDesigner)
                return;

            btnSlowMove2Nullpkt.Click += (s, e) => cfg.SendSlowMove(cmbStepDirection.SelectedIndex == 0);
            btnSetZero.Click += (s, e) => cfg.SetZeroPoint();
            btnSlowMove2End.Click += (s, e) => cfg.SetEndPoint();
            btnTest.Click += (s, e) => cfg.StartTestRun();
        }
        public override void LoadFromDecoder()
        {
            if (IsInDesigner)
                return;
            // Werte werden vom Decoder geladen, mit bis zu 3 Versuchen, um die Zuverlässigkeit zu erhöhen
            var s = cfg.GetSettingsWithRetry();

            if (s == null)
            {
                MessageBox.Show("Decoder antwortet nicht.");
                return;
            }
            // Werte werden in die Formularelemente übertragen
            numAddress.Value = s.Address;
            numDelay.Value = s.Delay;
            numStepsToEnd.Value = s.StepsToEnd;

            cmbRightLeft.SelectedIndex = s.RightOrLeft;
            cmbStepDirection.SelectedIndex = s.StepDirection;
        }

        public override void SaveToDecoder()
        {
            if (IsInDesigner)
                return;
            // Werte aus den Formularelementen werden in ein SwitchSettings-Objekt übertragen 
            var s = new SignalSettings
            {
                //    AssignedId = cfg.AssignedId,
                Address = (byte)numAddress.Value,
                Delay = (byte)numDelay.Value,
                StepsToEnd = (ushort)numStepsToEnd.Value,
                RightOrLeft = (byte)cmbRightLeft.SelectedIndex,
                StepDirection = (byte)cmbStepDirection.SelectedIndex
            };
            // und an den Decoder gesendet
            cfg.SetSettings(s);
        }
    }
}

