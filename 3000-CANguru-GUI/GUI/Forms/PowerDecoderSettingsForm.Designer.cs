using CANguru.GUI.Panels;
using CANguru.Views;
namespace CANguru.GUI.Forms
{
    partial class PowerDecoderSettingsForm
    {
        private System.ComponentModel.IContainer components = null;

        private Panel panelSettings;
        private Panel panelButtons;

        private Button btnOK;
        private Button btnCancel;

        private PowerDecoderSettingsPanel panel;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            panelSettings = new Panel();
            panel = new PowerDecoderSettingsPanel();
            panelButtons = new Panel();
            btnResetToDefaults = new Button();
            btnOK = new Button();
            btnCancel = new Button();
            panelSettings.SuspendLayout();
            panelButtons.SuspendLayout();
            SuspendLayout();
            // 
            // panelSettings
            // 
            panelSettings.Controls.Add(panel);
            panelSettings.Location = new Point(19, 8);
            panelSettings.Name = "panelSettings";
            panelSettings.Size = new Size(304, 70);
            panelSettings.TabIndex = 0;
            // 
            // panel
            // 
            panel.Location = new Point(0, 0);
            panel.Name = "panel";
            panel.Size = new Size(311, 60);
            panel.TabIndex = 0;
            // 
            // panelButtons
            // 
            panelButtons.Controls.Add(btnResetToDefaults);
            panelButtons.Controls.Add(btnOK);
            panelButtons.Controls.Add(btnCancel);
            panelButtons.Location = new Point(12, 61);
            panelButtons.Name = "panelButtons";
            panelButtons.Size = new Size(318, 40);
            panelButtons.TabIndex = 1;
            // 
            // btnResetToDefaults
            // 
            btnResetToDefaults.Location = new Point(11, 10);
            btnResetToDefaults.Name = "btnResetToDefaults";
            btnResetToDefaults.Size = new Size(106, 23);
            btnResetToDefaults.TabIndex = 2;
            btnResetToDefaults.Text = "Werkseinstellung";
            btnResetToDefaults.UseVisualStyleBackColor = true;
            btnResetToDefaults.Click += btnResetToDefaults_Click;
            // 
            // btnOK
            // 
            btnOK.Location = new Point(150, 8);
            btnOK.Name = "btnOK";
            btnOK.Size = new Size(75, 25);
            btnOK.TabIndex = 0;
            btnOK.Text = "Speichern";
            btnOK.Click += btnOK_Click;
            // 
            // btnCancel
            // 
            btnCancel.Location = new Point(235, 8);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(75, 25);
            btnCancel.TabIndex = 1;
            btnCancel.Text = "Schließen";
            btnCancel.Click += btnCancel_Click;
            // 
            // PowerDecoderSettingsForm
            // 
            ClientSize = new Size(353, 116);
            Controls.Add(panelButtons);
            Controls.Add(panelSettings);
            Name = "PowerDecoderSettingsForm";
            StartPosition = FormStartPosition.CenterParent;
            Text = "Power Decoder Einstellungen";
            Load += PowerDecoderSettingsForm_Load;
            panelSettings.ResumeLayout(false);
            panelButtons.ResumeLayout(false);
            ResumeLayout(false);
        }

        private Button btnResetToDefaults;
    }
}
