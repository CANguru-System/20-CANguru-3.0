using CANguru.GUI.Forms;
using CANguru.GUI.DecoderSettings;
using System.ComponentModel;
using System.Diagnostics;

namespace CANguru.GUI.Panels
{
    public partial class SwitchDecoderSettingsPanel : DecoderSettingsPanelBase
    {

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
        public SwitchDecoderSettingsPanel()
        {
            InitializeComponent();

            // statische UI (Texte etc.) DARF im Designer laufen
            InitStaticUI();

            // Layout NICHT hier – das macht die Designer-Datei

            if (!IsInDesigner)
                BuildRuntimeUI();   // nur Werte, keine Positionen
        }
    }
}

