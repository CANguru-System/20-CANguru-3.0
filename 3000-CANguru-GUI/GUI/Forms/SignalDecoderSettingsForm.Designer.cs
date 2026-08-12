using CANguru.GUI.Panels;
using CANguru.DecoderControls;
namespace CANguru.GUI.Forms
{
    partial class SignalDecoderSettingsForm
    {
        private System.ComponentModel.IContainer components = null;

        private Panel panelSettings;
        private Panel panelButtons;

        private Button btnOK;
        private Button btnCancel;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        public void InitializeComponent()
        {
            panelSettings = new Panel();
            panelButtons = new Panel();
            btnResetToDefaults = new Button();
            btnOK = new Button();
            btnCancel = new Button();
            signalDecoderSettingsPanel1 = new SignalDecoderSettingsPanel();
            panelSettings.SuspendLayout();
            panelButtons.SuspendLayout();
            SuspendLayout();
            // 
            // panelSettings
            // 
            panelSettings.Controls.Add(panelButtons);
            panelSettings.Controls.Add(signalDecoderSettingsPanel1);
            panelSettings.Location = new Point(12, 12);
            panelSettings.Name = "panelSettings";
            panelSettings.Size = new Size(360, 413);
            panelSettings.TabIndex = 1;
            // 
            // panelButtons
            // 
            panelButtons.Controls.Add(btnResetToDefaults);
            panelButtons.Controls.Add(btnOK);
            panelButtons.Controls.Add(btnCancel);
            panelButtons.Location = new Point(3, 372);
            panelButtons.Name = "panelButtons";
            panelButtons.Size = new Size(360, 40);
            panelButtons.TabIndex = 0;
            // 
            // btnResetToDefaults
            // 
            btnResetToDefaults.AutoSize = true;
            btnResetToDefaults.BackColor = SystemColors.Highlight;
            btnResetToDefaults.Location = new Point(17, 10);
            btnResetToDefaults.Name = "btnResetToDefaults";
            btnResetToDefaults.Size = new Size(123, 25);
            btnResetToDefaults.TabIndex = 2;
            btnResetToDefaults.TabStop = false;
            btnResetToDefaults.Text = "Werkseinstellungen";
            btnResetToDefaults.UseVisualStyleBackColor = true;
            btnResetToDefaults.Click += btnResetToDefaults_Click;
            // 
            // btnOK
            // 
            btnOK.Location = new Point(190, 8);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(75, 25);
            btnOK.TabIndex = 0;
            btnOK.Text = "Speichern";
            btnOK.Click += btnOK_Click;
            // 
            // btnCancel
            // 
            btnCancel.Location = new Point(275, 8);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(75, 25);
            btnCancel.TabIndex = 1;
            btnCancel.Text = "Schließen";
            btnCancel.Click += btnCancel_Click;
            // 
            // signalDecoderSettingsPanel1
            // 
            signalDecoderSettingsPanel1.Dock = DockStyle.Fill;
            signalDecoderSettingsPanel1.Location = new Point(0, 0);
            signalDecoderSettingsPanel1.Name = "signalDecoderSettingsPanel1";
            signalDecoderSettingsPanel1.Size = new Size(360, 413);
            signalDecoderSettingsPanel1.TabIndex = 0;
            // 
            // SignalDecoderSettingsForm
            // 
            ClientSize = new Size(384, 433);
            Controls.Add(panelSettings);
            Name = "SignalDecoderSettingsForm";
            StartPosition = FormStartPosition.CenterParent;
            Text = "Signal Decoder Einstellungen";
            Load += SignalDecoderSettingsForm_Load;
            panelSettings.ResumeLayout(false);
            panelButtons.ResumeLayout(false);
            panelButtons.PerformLayout();
            ResumeLayout(false);
        }

        private Button btnResetToDefaults;
        private SignalDecoderSettingsPanel signalDecoderSettingsPanel1;
    }
}
