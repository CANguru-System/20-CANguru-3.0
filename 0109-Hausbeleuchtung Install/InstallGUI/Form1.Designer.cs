using System.Windows.Forms;

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
            this.lightportsBox = new System.Windows.Forms.ListBox();
            this.scanPortsBtn = new System.Windows.Forms.Button();
            this.helpbtn = new System.Windows.Forms.Button();
            this.erasebtn = new System.Windows.Forms.Button();
            this.ipbox = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.reportBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.eyeicon = new System.Windows.Forms.PictureBox();
            this.label4 = new System.Windows.Forms.Label();
            this.processBox = new System.Windows.Forms.TextBox();
            this.transmitBtn = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.decoderBox = new System.Windows.Forms.ComboBox();
            this.raeume = new System.Windows.Forms.GroupBox();
            this.baseTime = new System.Windows.Forms.NumericUpDown();
            this.brightness = new System.Windows.Forms.NumericUpDown();
            this.offTime = new System.Windows.Forms.NumericUpDown();
            this.onTime = new System.Windows.Forms.NumericUpDown();
            this.comportsBox = new System.Windows.Forms.ListBox();
            this.label11 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.eyeicon)).BeginInit();
            this.raeume.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.baseTime)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.brightness)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.offTime)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.onTime)).BeginInit();
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
            this.label2.Location = new System.Drawing.Point(17, 320);
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
            this.password.Location = new System.Drawing.Point(20, 336);
            this.password.Name = "password";
            this.password.Size = new System.Drawing.Size(130, 20);
            this.password.TabIndex = 3;
            // 
            // saveBtn
            // 
            this.saveBtn.Location = new System.Drawing.Point(380, 299);
            this.saveBtn.Name = "saveBtn";
            this.saveBtn.Size = new System.Drawing.Size(75, 23);
            this.saveBtn.TabIndex = 4;
            this.saveBtn.Text = "Speichern";
            this.saveBtn.UseVisualStyleBackColor = true;
            this.saveBtn.Click += new System.EventHandler(this.saveBtn_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(380, 507);
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
            this.uploadBtn.Location = new System.Drawing.Point(380, 369);
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
            this.label3.Location = new System.Drawing.Point(17, 249);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(102, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "Beleuchtungs-Ports:";
            // 
            // lightportsBox
            // 
            this.lightportsBox.FormattingEnabled = true;
            this.lightportsBox.Location = new System.Drawing.Point(20, 264);
            this.lightportsBox.Name = "lightportsBox";
            this.lightportsBox.Size = new System.Drawing.Size(185, 43);
            this.lightportsBox.TabIndex = 9;
            this.lightportsBox.SelectedIndexChanged += new System.EventHandler(this.lightportsBox_SelectedIndexChanged);
            // 
            // scanPortsBtn
            // 
            this.scanPortsBtn.Location = new System.Drawing.Point(17, 219);
            this.scanPortsBtn.Name = "scanPortsBtn";
            this.scanPortsBtn.Size = new System.Drawing.Size(75, 23);
            this.scanPortsBtn.TabIndex = 10;
            this.scanPortsBtn.Text = "Scan Ports";
            this.scanPortsBtn.UseVisualStyleBackColor = true;
            this.scanPortsBtn.Click += new System.EventHandler(this.scanPortsBtn_Click);
            // 
            // helpbtn
            // 
            this.helpbtn.Location = new System.Drawing.Point(380, 470);
            this.helpbtn.Name = "helpbtn";
            this.helpbtn.Size = new System.Drawing.Size(75, 23);
            this.helpbtn.TabIndex = 14;
            this.helpbtn.Text = "Hilfe";
            this.helpbtn.UseVisualStyleBackColor = true;
            this.helpbtn.Click += new System.EventHandler(this.helpbtn_Click);
            // 
            // erasebtn
            // 
            this.erasebtn.Location = new System.Drawing.Point(380, 334);
            this.erasebtn.Name = "erasebtn";
            this.erasebtn.Size = new System.Drawing.Size(75, 23);
            this.erasebtn.TabIndex = 15;
            this.erasebtn.Text = "Erase flash";
            this.erasebtn.UseVisualStyleBackColor = true;
            this.erasebtn.Click += new System.EventHandler(this.erasebtn_Click);
            // 
            // ipbox
            // 
            this.ipbox.Location = new System.Drawing.Point(240, 336);
            this.ipbox.Name = "ipbox";
            this.ipbox.Size = new System.Drawing.Size(127, 20);
            this.ipbox.TabIndex = 16;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(240, 320);
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
            // eyeicon
            // 
            this.eyeicon.Image = ((System.Drawing.Image)(resources.GetObject("eyeicon.Image")));
            this.eyeicon.InitialImage = ((System.Drawing.Image)(resources.GetObject("eyeicon.InitialImage")));
            this.eyeicon.Location = new System.Drawing.Point(152, 324);
            this.eyeicon.Name = "eyeicon";
            this.eyeicon.Size = new System.Drawing.Size(25, 16);
            this.eyeicon.TabIndex = 22;
            this.eyeicon.TabStop = false;
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
            // transmitBtn
            // 
            this.transmitBtn.Location = new System.Drawing.Point(253, 503);
            this.transmitBtn.Name = "transmitBtn";
            this.transmitBtn.Size = new System.Drawing.Size(75, 23);
            this.transmitBtn.TabIndex = 23;
            this.transmitBtn.Text = "Save";
            this.transmitBtn.UseVisualStyleBackColor = true;
            this.transmitBtn.Click += new System.EventHandler(this.transmitBtn_Click);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(209, 470);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(48, 13);
            this.label7.TabIndex = 28;
            this.label7.Text = "Basiszeit";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(17, 435);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(42, 13);
            this.label8.TabIndex = 29;
            this.label8.Text = "onTime";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(17, 470);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(42, 13);
            this.label9.TabIndex = 30;
            this.label9.Text = "offTime";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(209, 439);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(50, 13);
            this.label10.TabIndex = 31;
            this.label10.Text = "Helligkeit";
            // 
            // decoderBox
            // 
            this.decoderBox.FormattingEnabled = true;
            this.decoderBox.Location = new System.Drawing.Point(17, 396);
            this.decoderBox.Name = "decoderBox";
            this.decoderBox.Size = new System.Drawing.Size(40, 21);
            this.decoderBox.TabIndex = 33;
            this.decoderBox.SelectedIndexChanged += new System.EventHandler(this.decoderBox_SelectedIndexChanged);
            // 
            // raeume
            // 
            this.raeume.Controls.Add(this.baseTime);
            this.raeume.Controls.Add(this.brightness);
            this.raeume.Controls.Add(this.offTime);
            this.raeume.Controls.Add(this.onTime);
            this.raeume.Location = new System.Drawing.Point(12, 369);
            this.raeume.Name = "raeume";
            this.raeume.Size = new System.Drawing.Size(332, 161);
            this.raeume.TabIndex = 35;
            this.raeume.TabStop = false;
            this.raeume.Text = "Räume";
            // 
            // baseTime
            // 
            this.baseTime.Location = new System.Drawing.Point(258, 104);
            this.baseTime.Name = "baseTime";
            this.baseTime.Size = new System.Drawing.Size(60, 20);
            this.baseTime.TabIndex = 3;
            this.baseTime.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // brightness
            // 
            this.brightness.Location = new System.Drawing.Point(260, 68);
            this.brightness.Name = "brightness";
            this.brightness.Size = new System.Drawing.Size(60, 20);
            this.brightness.TabIndex = 2;
            this.brightness.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // offTime
            // 
            this.offTime.Location = new System.Drawing.Point(69, 99);
            this.offTime.Name = "offTime";
            this.offTime.Size = new System.Drawing.Size(60, 20);
            this.offTime.TabIndex = 1;
            this.offTime.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // onTime
            // 
            this.onTime.Location = new System.Drawing.Point(69, 64);
            this.onTime.Name = "onTime";
            this.onTime.Size = new System.Drawing.Size(60, 20);
            this.onTime.TabIndex = 0;
            this.onTime.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // comportsBox
            // 
            this.comportsBox.FormattingEnabled = true;
            this.comportsBox.Location = new System.Drawing.Point(20, 170);
            this.comportsBox.Name = "comportsBox";
            this.comportsBox.Size = new System.Drawing.Size(185, 43);
            this.comportsBox.TabIndex = 36;
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(20, 154);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(58, 13);
            this.label11.TabIndex = 37;
            this.label11.Text = "COM-Ports";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(479, 538);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.comportsBox);
            this.Controls.Add(this.decoderBox);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.transmitBtn);
            this.Controls.Add(this.eyeicon);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.reportBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.ipbox);
            this.Controls.Add(this.erasebtn);
            this.Controls.Add(this.helpbtn);
            this.Controls.Add(this.processBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.scanPortsBtn);
            this.Controls.Add(this.lightportsBox);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.uploadBtn);
            this.Controls.Add(this.scanSSID);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.saveBtn);
            this.Controls.Add(this.password);
            this.Controls.Add(this.ssidBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.raeume);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "Hausbeleuchtung-Installationsprogramm 1.0";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.eyeicon)).EndInit();
            this.raeume.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.baseTime)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.brightness)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.offTime)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.onTime)).EndInit();
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
        private System.Windows.Forms.ListBox lightportsBox;
        private System.Windows.Forms.Button scanPortsBtn;
        private System.Windows.Forms.Button helpbtn;
        private System.Windows.Forms.Button erasebtn;
        private System.Windows.Forms.TextBox ipbox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox reportBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.PictureBox eyeicon;
        private Label label4;
        private TextBox processBox;
        private Button transmitBtn;
        private Label label7;
        private Label label8;
        private Label label9;
        private Label label10;
        private ComboBox decoderBox;
        private GroupBox raeume;
        private NumericUpDown onTime;
        private NumericUpDown baseTime;
        private NumericUpDown brightness;
        private NumericUpDown offTime;
        private ListBox comportsBox;
        private Label label11;
    }
}

