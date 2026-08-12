namespace FirmwareFlasher
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        private System.Windows.Forms.TextBox txtFirmware;
        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.ComboBox cmbPorts;
        private System.Windows.Forms.Button btnFlash;
        private System.Windows.Forms.TextBox txtLog;

        private void InitializeComponent()
        {
            txtFirmware = new TextBox();
            btnBrowse = new Button();
            cmbPorts = new ComboBox();
            btnFlash = new Button();
            txtLog = new TextBox();
            btnEraseFlash = new Button();
            SuspendLayout();
            // 
            // txtFirmware
            // 
            txtFirmware.Location = new Point(12, 12);
            txtFirmware.Name = "txtFirmware";
            txtFirmware.Size = new Size(288, 23);
            txtFirmware.TabIndex = 0;
            // 
            // btnBrowse
            // 
            btnBrowse.Location = new Point(311, 10);
            btnBrowse.Name = "btnBrowse";
            btnBrowse.Size = new Size(89, 23);
            btnBrowse.TabIndex = 1;
            btnBrowse.Text = "Durchsuchen";
            btnBrowse.Click += btnBrowse_Click;
            // 
            // cmbPorts
            // 
            cmbPorts.Location = new Point(12, 50);
            cmbPorts.Name = "cmbPorts";
            cmbPorts.Size = new Size(150, 23);
            cmbPorts.TabIndex = 2;
            // 
            // btnFlash
            // 
            btnFlash.Location = new Point(311, 50);
            btnFlash.Name = "btnFlash";
            btnFlash.Size = new Size(75, 23);
            btnFlash.TabIndex = 3;
            btnFlash.Text = "Flash starten";
            btnFlash.Click += btnFlash_Click;
            // 
            // txtLog
            // 
            txtLog.Location = new Point(12, 90);
            txtLog.Multiline = true;
            txtLog.Name = "txtLog";
            txtLog.ScrollBars = ScrollBars.Vertical;
            txtLog.Size = new Size(380, 200);
            txtLog.TabIndex = 4;
            // 
            // btnEraseFlash
            // 
            btnEraseFlash.Location = new Point(185, 50);
            btnEraseFlash.Name = "btnEraseFlash";
            btnEraseFlash.Size = new Size(75, 23);
            btnEraseFlash.TabIndex = 5;
            btnEraseFlash.Text = "Erase Flash";
            btnEraseFlash.UseVisualStyleBackColor = true;
            btnEraseFlash.Click += btnEraseFlash_Click;
            // 
            // Form1
            // 
            ClientSize = new Size(410, 310);
            Controls.Add(btnEraseFlash);
            Controls.Add(txtFirmware);
            Controls.Add(btnBrowse);
            Controls.Add(cmbPorts);
            Controls.Add(btnFlash);
            Controls.Add(txtLog);
            Name = "Form1";
            Text = "Decoder Firmware Flasher";
            ResumeLayout(false);
            PerformLayout();
        }

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>

        #endregion

        private Button btnEraseFlash;
    }
}
