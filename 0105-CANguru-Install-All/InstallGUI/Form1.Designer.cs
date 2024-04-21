namespace InstallGUI
{
    partial class Form1
    {
        /// <summary>
        /// Erforderliche Designervariable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Verwendete Ressourcen bereinigen.
        /// </summary>
        /// <param name="disposing">True, wenn verwaltete Ressourcen gelöscht werden sollen; andernfalls False.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Vom Windows Form-Designer generierter Code

        /// <summary>
        /// Erforderliche Methode für die Designerunterstützung.
        /// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.ssidBox = new System.Windows.Forms.ListBox();
            this.password = new System.Windows.Forms.TextBox();
            this.saveBtn = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.scanSSID = new System.Windows.Forms.Button();
            this.uploadBtn = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.comportsBox = new System.Windows.Forms.ListBox();
            this.scanPortsBtn = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.processBox = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.rbBridge = new System.Windows.Forms.RadioButton();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.rbBooster = new System.Windows.Forms.RadioButton();
            this.rbstepper = new System.Windows.Forms.RadioButton();
            this.rbgleisbesetztmelder = new System.Windows.Forms.RadioButton();
            this.helpbtn = new System.Windows.Forms.Button();
            this.erasebtn = new System.Windows.Forms.Button();
            this.ipbox = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.reportBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(267, 153);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(61, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Netzwerke:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(17, 297);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(50, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Passwort";
            // 
            // ssidBox
            // 
            this.ssidBox.FormattingEnabled = true;
            this.ssidBox.Location = new System.Drawing.Point(270, 172);
            this.ssidBox.Name = "ssidBox";
            this.ssidBox.Size = new System.Drawing.Size(185, 82);
            this.ssidBox.TabIndex = 2;
            // 
            // password
            // 
            this.password.Location = new System.Drawing.Point(20, 322);
            this.password.Name = "password";
            this.password.Size = new System.Drawing.Size(130, 20);
            this.password.TabIndex = 3;
            // 
            // saveBtn
            // 
            this.saveBtn.Location = new System.Drawing.Point(380, 303);
            this.saveBtn.Name = "saveBtn";
            this.saveBtn.Size = new System.Drawing.Size(75, 23);
            this.saveBtn.TabIndex = 4;
            this.saveBtn.Text = "Speichern";
            this.saveBtn.UseVisualStyleBackColor = true;
            this.saveBtn.Click += new System.EventHandler(this.saveBtn_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(380, 497);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 5;
            this.button2.Text = "Beenden";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.finishBtn_Click);
            // 
            // scanSSID
            // 
            this.scanSSID.Location = new System.Drawing.Point(380, 264);
            this.scanSSID.Name = "scanSSID";
            this.scanSSID.Size = new System.Drawing.Size(75, 23);
            this.scanSSID.TabIndex = 6;
            this.scanSSID.Text = "Scan SSIDs";
            this.scanSSID.UseVisualStyleBackColor = true;
            this.scanSSID.Click += new System.EventHandler(this.scanSSIDBtn_Click);
            // 
            // uploadBtn
            // 
            this.uploadBtn.Location = new System.Drawing.Point(253, 497);
            this.uploadBtn.Name = "uploadBtn";
            this.uploadBtn.Size = new System.Drawing.Size(75, 23);
            this.uploadBtn.TabIndex = 7;
            this.uploadBtn.Text = "Upload";
            this.uploadBtn.UseVisualStyleBackColor = true;
            this.uploadBtn.Click += new System.EventHandler(this.uploadBtn_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(17, 153);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(61, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "COM-Ports:";
            // 
            // comportsBox
            // 
            this.comportsBox.FormattingEnabled = true;
            this.comportsBox.Location = new System.Drawing.Point(20, 172);
            this.comportsBox.Name = "comportsBox";
            this.comportsBox.Size = new System.Drawing.Size(185, 82);
            this.comportsBox.TabIndex = 9;
            this.comportsBox.SelectedIndexChanged += new System.EventHandler(this.comportsBox_SelectedIndexChanged);
            // 
            // scanPortsBtn
            // 
            this.scanPortsBtn.Location = new System.Drawing.Point(23, 264);
            this.scanPortsBtn.Name = "scanPortsBtn";
            this.scanPortsBtn.Size = new System.Drawing.Size(75, 23);
            this.scanPortsBtn.TabIndex = 10;
            this.scanPortsBtn.Text = "Scan Ports";
            this.scanPortsBtn.UseVisualStyleBackColor = true;
            this.scanPortsBtn.Click += new System.EventHandler(this.scanPortsBtn_Click);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(20, 9);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(56, 13);
            this.label4.TabIndex = 11;
            this.label4.Text = "Fortschritt:";
            // 
            // processBox
            // 
            this.processBox.Location = new System.Drawing.Point(23, 25);
            this.processBox.Multiline = true;
            this.processBox.Name = "processBox";
            this.processBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.processBox.Size = new System.Drawing.Size(435, 64);
            this.processBox.TabIndex = 12;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.rbBridge);
            this.groupBox1.Controls.Add(this.radioButton1);
            this.groupBox1.Controls.Add(this.rbBooster);
            this.groupBox1.Controls.Add(this.rbstepper);
            this.groupBox1.Controls.Add(this.rbgleisbesetztmelder);
            this.groupBox1.Location = new System.Drawing.Point(20, 362);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(354, 129);
            this.groupBox1.TabIndex = 13;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "processorBox";
            // 
            // rbBridge
            // 
            this.rbBridge.AutoSize = true;
            this.rbBridge.Location = new System.Drawing.Point(18, 75);
            this.rbBridge.Name = "rbBridge";
            this.rbBridge.Size = new System.Drawing.Size(101, 17);
            this.rbBridge.TabIndex = 3;
            this.rbBridge.TabStop = true;
            this.rbBridge.Text = "CANguru-Bridge";
            this.rbBridge.UseVisualStyleBackColor = true;
            this.rbBridge.CheckedChanged += new System.EventHandler(this.rbBridge_CheckedChanged);
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Location = new System.Drawing.Point(220, 23);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(112, 17);
            this.radioButton1.TabIndex = 2;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "Nächster Decoder";
            this.radioButton1.UseVisualStyleBackColor = true;
            // 
            // rbBooster
            // 
            this.rbBooster.AutoSize = true;
            this.rbBooster.Location = new System.Drawing.Point(18, 101);
            this.rbBooster.Name = "rbBooster";
            this.rbBooster.Size = new System.Drawing.Size(107, 17);
            this.rbBooster.TabIndex = 2;
            this.rbBooster.TabStop = true;
            this.rbBooster.Text = "CANguru-Booster";
            this.rbBooster.UseVisualStyleBackColor = true;
            this.rbBooster.CheckedChanged += new System.EventHandler(this.rbBooster_CheckedChanged);
            // 
            // rbstepper
            // 
            this.rbstepper.AutoSize = true;
            this.rbstepper.Location = new System.Drawing.Point(18, 49);
            this.rbstepper.Name = "rbstepper";
            this.rbstepper.Size = new System.Drawing.Size(166, 17);
            this.rbstepper.TabIndex = 1;
            this.rbstepper.TabStop = true;
            this.rbstepper.Text = "Stepperweiche auf ESP32-C3";
            this.rbstepper.UseVisualStyleBackColor = true;
            this.rbstepper.CheckedChanged += new System.EventHandler(this.rbstepper_CheckedChanged);
            // 
            // rbgleisbesetztmelder
            // 
            this.rbgleisbesetztmelder.AutoSize = true;
            this.rbgleisbesetztmelder.Location = new System.Drawing.Point(18, 23);
            this.rbgleisbesetztmelder.Name = "rbgleisbesetztmelder";
            this.rbgleisbesetztmelder.Size = new System.Drawing.Size(167, 17);
            this.rbgleisbesetztmelder.TabIndex = 0;
            this.rbgleisbesetztmelder.TabStop = true;
            this.rbgleisbesetztmelder.Text = "Gleisbesetztmelder auf ESP32";
            this.rbgleisbesetztmelder.UseVisualStyleBackColor = true;
            this.rbgleisbesetztmelder.CheckedChanged += new System.EventHandler(this.rbgleisbesetztmelder_CheckedChanged);
            // 
            // helpbtn
            // 
            this.helpbtn.Location = new System.Drawing.Point(380, 438);
            this.helpbtn.Name = "helpbtn";
            this.helpbtn.Size = new System.Drawing.Size(75, 23);
            this.helpbtn.TabIndex = 14;
            this.helpbtn.Text = "Hilfe";
            this.helpbtn.UseVisualStyleBackColor = true;
            this.helpbtn.Click += new System.EventHandler(this.helpbtn_Click);
            // 
            // erasebtn
            // 
            this.erasebtn.Location = new System.Drawing.Point(380, 342);
            this.erasebtn.Name = "erasebtn";
            this.erasebtn.Size = new System.Drawing.Size(75, 23);
            this.erasebtn.TabIndex = 15;
            this.erasebtn.Text = "Erase flash";
            this.erasebtn.UseVisualStyleBackColor = true;
            this.erasebtn.Click += new System.EventHandler(this.erasebtn_Click);
            // 
            // ipbox
            // 
            this.ipbox.Location = new System.Drawing.Point(201, 322);
            this.ipbox.Name = "ipbox";
            this.ipbox.Size = new System.Drawing.Size(127, 20);
            this.ipbox.TabIndex = 16;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(201, 302);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(57, 13);
            this.label5.TabIndex = 17;
            this.label5.Text = "IP-adresse";
            // 
            // reportBox
            // 
            this.reportBox.Location = new System.Drawing.Point(26, 119);
            this.reportBox.Name = "reportBox";
            this.reportBox.Size = new System.Drawing.Size(429, 20);
            this.reportBox.TabIndex = 18;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(26, 100);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(60, 13);
            this.label6.TabIndex = 19;
            this.label6.Text = "Meldungen";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(483, 524);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.reportBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.ipbox);
            this.Controls.Add(this.erasebtn);
            this.Controls.Add(this.helpbtn);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.processBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.scanPortsBtn);
            this.Controls.Add(this.comportsBox);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.uploadBtn);
            this.Controls.Add(this.scanSSID);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.saveBtn);
            this.Controls.Add(this.password);
            this.Controls.Add(this.ssidBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "CANguru-Installationsprogramm 1.0";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ListBox ssidBox;
        private System.Windows.Forms.TextBox password;
        private System.Windows.Forms.Button saveBtn;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button scanSSID;
        private System.Windows.Forms.Button uploadBtn;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ListBox comportsBox;
        private System.Windows.Forms.Button scanPortsBtn;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox processBox;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton rbgleisbesetztmelder;
        private System.Windows.Forms.RadioButton rbstepper;
        private System.Windows.Forms.RadioButton rbBooster;
        private System.Windows.Forms.Button helpbtn;
        private System.Windows.Forms.Button erasebtn;
        private System.Windows.Forms.RadioButton rbBridge;
        private System.Windows.Forms.TextBox ipbox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox reportBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.RadioButton radioButton1;
    }
}

