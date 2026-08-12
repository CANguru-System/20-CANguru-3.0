namespace CANguru.Views
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

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
        private System.Windows.Forms.Panel headerPanel;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnMinimize;
        private System.Windows.Forms.Label lblTitle;

        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();
            lblTitle = new Label();
            headerPanel = new Panel();
            pictureBox1 = new PictureBox();
            btnClose = new Button();
            btnMinimize = new Button();
            tabPage1 = new TabPage();
            showDecoderList = new Button();
            buttonSettings = new Button();
            btnOtaUpdate = new Button();
            listBoxDecoder = new ListBox();
            radioType1 = new RadioButton();
            radioType2 = new RadioButton();
            radioType3 = new RadioButton();
            radioType4 = new RadioButton();
            radioSingle = new RadioButton();
            tabPage2 = new TabPage();
            showLokList = new Button();
            listBoxMFX = new TextBox();
            UpDnnextLokAddress = new NumericUpDown();
            UpDnnewSubscriptionCounter = new NumericUpDown();
            label1 = new Label();
            label2 = new Label();
            btnLokListe = new Button();
            btnCANguruLoks = new Button();
            btnDelete = new Button();
            btnAddManualLok = new Button();
            tabMain = new TabPage();
            listBoxCAN = new TextBox();
            listBoxLoks = new ListBox();
            tbMessages = new TextBox();
            txtBridgeIP = new TextBox();
            groupBox2 = new GroupBox();
            groupBox3 = new GroupBox();
            btnConnect = new Button();
            label3 = new Label();
            lblBRIDGEStatus = new Label();
            btnExit = new Button();
            tabControl1 = new TabControl();
            canTimer = new System.Windows.Forms.Timer(components);
            aliveTimer = new System.Windows.Forms.Timer(components);
            btnCopy = new Button();
            PINGtimer = new System.Windows.Forms.Timer(components);
            looptimer = new System.Windows.Forms.Timer(components);
            STOPP_Timer = new System.Windows.Forms.Timer(components);
            labelCurrent0 = new TextBox();
            labelCurrent1 = new TextBox();
            panelCurrent0 = new Panel();
            panelCurrent1 = new Panel();
            cntPower = new TextBox();
            cntSwitch = new TextBox();
            cntSignal = new TextBox();
            cntFeedback = new TextBox();
            label4 = new Label();
            label5 = new Label();
            label6 = new Label();
            label7 = new Label();
            headerPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)pictureBox1).BeginInit();
            tabPage1.SuspendLayout();
            tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)UpDnnextLokAddress).BeginInit();
            ((System.ComponentModel.ISupportInitialize)UpDnnewSubscriptionCounter).BeginInit();
            tabMain.SuspendLayout();
            groupBox3.SuspendLayout();
            tabControl1.SuspendLayout();
            panelCurrent0.SuspendLayout();
            panelCurrent1.SuspendLayout();
            SuspendLayout();
            // 
            // lblTitle
            // 
            lblTitle.AutoSize = true;
            lblTitle.Font = new Font("Segoe UI", 11F, FontStyle.Bold);
            lblTitle.ForeColor = Color.FromArgb(40, 40, 40);
            lblTitle.Location = new Point(45, 10);
            lblTitle.Name = "lblTitle";
            lblTitle.Size = new Size(156, 20);
            lblTitle.TabIndex = 2;
            lblTitle.Text = "CANguru Version 1.0";
            // 
            // headerPanel
            // 
            headerPanel.BackColor = Color.FromArgb(235, 235, 235);
            headerPanel.Controls.Add(pictureBox1);
            headerPanel.Controls.Add(btnClose);
            headerPanel.Controls.Add(btnMinimize);
            headerPanel.Controls.Add(lblTitle);
            headerPanel.Dock = DockStyle.Top;
            headerPanel.Location = new Point(0, 0);
            headerPanel.Name = "headerPanel";
            headerPanel.Size = new Size(604, 40);
            headerPanel.TabIndex = 0;
            headerPanel.MouseDown += headerPanel_MouseDown;
            // 
            // pictureBox1
            // 
            pictureBox1.Location = new Point(4, 0);
            pictureBox1.Name = "pictureBox1";
            pictureBox1.Size = new Size(45, 41);
            pictureBox1.TabIndex = 25;
            pictureBox1.TabStop = false;
            // 
            // btnClose
            // 
            btnClose.BackColor = Color.FromArgb(235, 235, 235);
            btnClose.Dock = DockStyle.Right;
            btnClose.FlatAppearance.BorderSize = 0;
            btnClose.FlatStyle = FlatStyle.Flat;
            btnClose.ForeColor = Color.FromArgb(50, 50, 50);
            btnClose.Location = new Point(524, 0);
            btnClose.Name = "btnClose";
            btnClose.Size = new Size(40, 40);
            btnClose.TabIndex = 0;
            btnClose.Text = "✕";
            btnClose.UseVisualStyleBackColor = false;
            btnClose.Click += btnClose_Click;
            // 
            // btnMinimize
            // 
            btnMinimize.BackColor = Color.FromArgb(235, 235, 235);
            btnMinimize.Dock = DockStyle.Right;
            btnMinimize.FlatAppearance.BorderSize = 0;
            btnMinimize.FlatStyle = FlatStyle.Flat;
            btnMinimize.ForeColor = Color.FromArgb(50, 50, 50);
            btnMinimize.Location = new Point(564, 0);
            btnMinimize.Name = "btnMinimize";
            btnMinimize.Size = new Size(40, 40);
            btnMinimize.TabIndex = 1;
            btnMinimize.Text = "–";
            btnMinimize.UseVisualStyleBackColor = false;
            btnMinimize.Click += btnMinimize_Click;
            // 
            // tabPage1
            // 
            tabPage1.Controls.Add(showDecoderList);
            tabPage1.Controls.Add(buttonSettings);
            tabPage1.Controls.Add(btnOtaUpdate);
            tabPage1.Controls.Add(listBoxDecoder);
            tabPage1.Controls.Add(radioType1);
            tabPage1.Controls.Add(radioType2);
            tabPage1.Controls.Add(radioType3);
            tabPage1.Controls.Add(radioType4);
            tabPage1.Controls.Add(radioSingle);
            tabPage1.Location = new Point(4, 4);
            tabPage1.Name = "tabPage1";
            tabPage1.Padding = new Padding(3);
            tabPage1.Size = new Size(485, 335);
            tabPage1.TabIndex = 2;
            tabPage1.Text = "Decoder";
            tabPage1.UseVisualStyleBackColor = true;
            // 
            // showDecoderList
            // 
            showDecoderList.Location = new Point(6, 268);
            showDecoderList.Name = "showDecoderList";
            showDecoderList.Size = new Size(144, 23);
            showDecoderList.TabIndex = 16;
            showDecoderList.Text = "Decoder-Liste anzeigen";
            showDecoderList.UseVisualStyleBackColor = true;
            showDecoderList.Click += showDecoderList_Click;
            // 
            // buttonSettings
            // 
            buttonSettings.Location = new Point(377, 268);
            buttonSettings.Name = "buttonSettings";
            buttonSettings.Size = new Size(95, 23);
            buttonSettings.TabIndex = 15;
            buttonSettings.Text = "Einstellungen";
            buttonSettings.UseVisualStyleBackColor = true;
            buttonSettings.Click += buttonSettings_Click;
            // 
            // btnOtaUpdate
            // 
            btnOtaUpdate.Location = new Point(377, 226);
            btnOtaUpdate.Name = "btnOtaUpdate";
            btnOtaUpdate.Size = new Size(95, 23);
            btnOtaUpdate.TabIndex = 1;
            btnOtaUpdate.Text = "OTA";
            btnOtaUpdate.UseVisualStyleBackColor = true;
            btnOtaUpdate.Click += btnOtaUpdate_Click;
            // 
            // listBoxDecoder
            // 
            listBoxDecoder.DrawMode = DrawMode.OwnerDrawFixed;
            listBoxDecoder.Font = new Font("Consolas", 10F);
            listBoxDecoder.FormattingEnabled = true;
            listBoxDecoder.ItemHeight = 18;
            listBoxDecoder.Location = new Point(6, 64);
            listBoxDecoder.Name = "listBoxDecoder";
            listBoxDecoder.Size = new Size(473, 130);
            listBoxDecoder.TabIndex = 0;
            listBoxDecoder.DrawItem += ListBoxDecoder_DrawItem;
            // 
            // radioType1
            // 
            radioType1.AutoSize = true;
            radioType1.Location = new Point(10, 205);
            radioType1.Name = "radioType1";
            radioType1.Size = new Size(114, 19);
            radioType1.TabIndex = 10;
            radioType1.TabStop = true;
            radioType1.Text = "Weichendecoder";
            radioType1.UseVisualStyleBackColor = true;
            // 
            // radioType2
            // 
            radioType2.AutoSize = true;
            radioType2.Location = new Point(10, 230);
            radioType2.Name = "radioType2";
            radioType2.Size = new Size(100, 19);
            radioType2.TabIndex = 11;
            radioType2.TabStop = true;
            radioType2.Text = "Signaldecoder";
            radioType2.UseVisualStyleBackColor = true;
            // 
            // radioType3
            // 
            radioType3.AutoSize = true;
            radioType3.Location = new Point(132, 205);
            radioType3.Name = "radioType3";
            radioType3.Size = new Size(65, 19);
            radioType3.TabIndex = 12;
            radioType3.TabStop = true;
            radioType3.Text = "Booster";
            radioType3.UseVisualStyleBackColor = true;
            // 
            // radioType4
            // 
            radioType4.AutoSize = true;
            radioType4.Location = new Point(132, 230);
            radioType4.Name = "radioType4";
            radioType4.Size = new Size(124, 19);
            radioType4.TabIndex = 13;
            radioType4.TabStop = true;
            radioType4.Text = "Gleisbesetztmelder";
            radioType4.UseVisualStyleBackColor = true;
            // 
            // radioSingle
            // 
            radioSingle.AutoSize = true;
            radioSingle.Location = new Point(264, 200);
            radioSingle.Name = "radioSingle";
            radioSingle.Size = new Size(123, 19);
            radioSingle.TabIndex = 14;
            radioSingle.TabStop = true;
            radioSingle.Text = "Einzeldecoder OTA";
            radioSingle.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            tabPage2.Controls.Add(showLokList);
            tabPage2.Controls.Add(listBoxMFX);
            tabPage2.Controls.Add(UpDnnextLokAddress);
            tabPage2.Controls.Add(UpDnnewSubscriptionCounter);
            tabPage2.Controls.Add(label1);
            tabPage2.Controls.Add(label2);
            tabPage2.Controls.Add(btnLokListe);
            tabPage2.Controls.Add(btnCANguruLoks);
            tabPage2.Controls.Add(btnDelete);
            tabPage2.Controls.Add(btnAddManualLok);
            tabPage2.Location = new Point(4, 4);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new Padding(3);
            tabPage2.Size = new Size(485, 335);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "Lok-Ctrl";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // showLokList
            // 
            showLokList.Location = new Point(225, 171);
            showLokList.Name = "showLokList";
            showLokList.Size = new Size(126, 23);
            showLokList.TabIndex = 18;
            showLokList.Text = "Lok-Liste anzeigen";
            showLokList.UseVisualStyleBackColor = true;
            showLokList.Click += btnShowLokList_Click;
            // 
            // listBoxMFX
            // 
            listBoxMFX.Location = new Point(5, -3);
            listBoxMFX.Multiline = true;
            listBoxMFX.Name = "listBoxMFX";
            listBoxMFX.Size = new Size(474, 99);
            listBoxMFX.TabIndex = 17;
            // 
            // UpDnnextLokAddress
            // 
            UpDnnextLokAddress.Location = new Point(158, 120);
            UpDnnextLokAddress.Name = "UpDnnextLokAddress";
            UpDnnextLokAddress.Size = new Size(40, 23);
            UpDnnextLokAddress.TabIndex = 10;
            UpDnnextLokAddress.ValueChanged += UpDnnextLokAddress_ValueChanged;
            // 
            // UpDnnewSubscriptionCounter
            // 
            UpDnnewSubscriptionCounter.Location = new Point(41, 120);
            UpDnnewSubscriptionCounter.Name = "UpDnnewSubscriptionCounter";
            UpDnnewSubscriptionCounter.Size = new Size(40, 23);
            UpDnnewSubscriptionCounter.TabIndex = 9;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(20, 102);
            label1.Name = "label1";
            label1.Size = new Size(106, 15);
            label1.TabIndex = 11;
            label1.Text = "Neuanmeldezähler";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(146, 102);
            label2.Name = "label2";
            label2.Size = new Size(72, 15);
            label2.TabIndex = 12;
            label2.Text = "Lok-Adresse";
            // 
            // btnLokListe
            // 
            btnLokListe.Location = new Point(355, 100);
            btnLokListe.Name = "btnLokListe";
            btnLokListe.Size = new Size(130, 23);
            btnLokListe.TabIndex = 14;
            btnLokListe.Text = "Lok-Liste anlegen";
            btnLokListe.UseVisualStyleBackColor = true;
            btnLokListe.Click += btnLokListe_Click;
            // 
            // btnCANguruLoks
            // 
            btnCANguruLoks.Location = new Point(221, 130);
            btnCANguruLoks.Name = "btnCANguruLoks";
            btnCANguruLoks.Size = new Size(130, 23);
            btnCANguruLoks.TabIndex = 13;
            btnCANguruLoks.Text = "Mfx Lok erfassen";
            btnCANguruLoks.UseVisualStyleBackColor = true;
            btnCANguruLoks.Click += btnCANguruLoks_Click_1;
            // 
            // btnDelete
            // 
            btnDelete.Location = new Point(221, 100);
            btnDelete.Name = "btnDelete";
            btnDelete.Size = new Size(130, 23);
            btnDelete.TabIndex = 15;
            btnDelete.Text = "Lok Löschen";
            btnDelete.UseVisualStyleBackColor = true;
            btnDelete.Click += btnDelete_Click;
            // 
            // btnAddManualLok
            // 
            btnAddManualLok.Location = new Point(355, 130);
            btnAddManualLok.Name = "btnAddManualLok";
            btnAddManualLok.Size = new Size(130, 23);
            btnAddManualLok.TabIndex = 16;
            btnAddManualLok.Text = "Lok manuell erfassen";
            btnAddManualLok.UseVisualStyleBackColor = true;
            btnAddManualLok.Click += btnAddManualLok_Click;
            // 
            // tabMain
            // 
            tabMain.Controls.Add(listBoxCAN);
            tabMain.Location = new Point(4, 4);
            tabMain.Name = "tabMain";
            tabMain.Padding = new Padding(3);
            tabMain.Size = new Size(485, 335);
            tabMain.TabIndex = 0;
            tabMain.Text = "CAN-Msg";
            tabMain.UseVisualStyleBackColor = true;
            // 
            // listBoxCAN
            // 
            listBoxCAN.AcceptsReturn = true;
            listBoxCAN.Font = new Font("Courier New", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            listBoxCAN.Location = new Point(9, 6);
            listBoxCAN.Multiline = true;
            listBoxCAN.Name = "listBoxCAN";
            listBoxCAN.Size = new Size(466, 326);
            listBoxCAN.TabIndex = 0;
            // 
            // listBoxLoks
            // 
            listBoxLoks.DrawMode = DrawMode.OwnerDrawFixed;
            listBoxLoks.Font = new Font("Consolas", 10F);
            listBoxLoks.FormattingEnabled = true;
            listBoxLoks.ItemHeight = 18;
            listBoxLoks.Location = new Point(7, 159);
            listBoxLoks.Name = "listBoxLoks";
            listBoxLoks.Size = new Size(485, 148);
            listBoxLoks.TabIndex = 10;
            listBoxLoks.DrawItem += listBoxLoks_DrawItem;
            // 
            // tbMessages
            // 
            tbMessages.Location = new Point(7, 47);
            tbMessages.Multiline = true;
            tbMessages.Name = "tbMessages";
            tbMessages.Size = new Size(484, 107);
            tbMessages.TabIndex = 9;
            tbMessages.Text = "Zum Starten bitte 'Verbinden' klicken\r\n";
            // 
            // txtBridgeIP
            // 
            txtBridgeIP.Location = new Point(507, 155);
            txtBridgeIP.Name = "txtBridgeIP";
            txtBridgeIP.Size = new Size(87, 23);
            txtBridgeIP.TabIndex = 6;
            txtBridgeIP.TextAlign = HorizontalAlignment.Center;
            // 
            // groupBox2
            // 
            groupBox2.Location = new Point(499, 135);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new Size(100, 40);
            groupBox2.TabIndex = 8;
            groupBox2.TabStop = false;
            groupBox2.Text = "Bridge Address";
            // 
            // groupBox3
            // 
            groupBox3.Controls.Add(btnConnect);
            groupBox3.Location = new Point(497, 37);
            groupBox3.Name = "groupBox3";
            groupBox3.Size = new Size(105, 57);
            groupBox3.TabIndex = 0;
            groupBox3.TabStop = false;
            groupBox3.Text = "Action";
            // 
            // btnConnect
            // 
            btnConnect.BackColor = Color.FromArgb(192, 255, 192);
            btnConnect.Location = new Point(9, 16);
            btnConnect.Name = "btnConnect";
            btnConnect.Size = new Size(90, 30);
            btnConnect.TabIndex = 3;
            btnConnect.Text = "Verbinden";
            btnConnect.UseVisualStyleBackColor = false;
            btnConnect.Click += btnConnect_Click;
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Location = new Point(509, 88);
            label3.Name = "label3";
            label3.Size = new Size(78, 15);
            label3.TabIndex = 10;
            label3.Text = "Bridge-Status";
            // 
            // lblBRIDGEStatus
            // 
            lblBRIDGEStatus.BackColor = Color.Red;
            lblBRIDGEStatus.Font = new Font("Segoe UI", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
            lblBRIDGEStatus.ForeColor = Color.White;
            lblBRIDGEStatus.Location = new Point(506, 107);
            lblBRIDGEStatus.Name = "lblBRIDGEStatus";
            lblBRIDGEStatus.Size = new Size(88, 20);
            lblBRIDGEStatus.TabIndex = 9;
            lblBRIDGEStatus.Text = "OFFLINE";
            lblBRIDGEStatus.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // btnExit
            // 
            btnExit.Location = new Point(513, 644);
            btnExit.Name = "btnExit";
            btnExit.Size = new Size(75, 23);
            btnExit.TabIndex = 5;
            btnExit.Text = "Beenden";
            btnExit.UseVisualStyleBackColor = true;
            btnExit.Click += btnExit_Click;
            // 
            // tabControl1
            // 
            tabControl1.Alignment = TabAlignment.Bottom;
            tabControl1.Controls.Add(tabMain);
            tabControl1.Controls.Add(tabPage2);
            tabControl1.Controls.Add(tabPage1);
            tabControl1.Location = new Point(3, 313);
            tabControl1.Name = "tabControl1";
            tabControl1.SelectedIndex = 0;
            tabControl1.Size = new Size(493, 363);
            tabControl1.SizeMode = TabSizeMode.Fixed;
            tabControl1.TabIndex = 7;
            // 
            // canTimer
            // 
            canTimer.Tick += canTimer_Tick;
            // 
            // aliveTimer
            // 
            aliveTimer.Enabled = true;
            aliveTimer.Interval = 500;
            aliveTimer.Tick += aliveTimer_Tick;
            // 
            // btnCopy
            // 
            btnCopy.Location = new Point(513, 606);
            btnCopy.Name = "btnCopy";
            btnCopy.Size = new Size(75, 23);
            btnCopy.TabIndex = 8;
            btnCopy.Text = "Copy";
            btnCopy.UseVisualStyleBackColor = true;
            btnCopy.Click += btnCopy_Click;
            // 
            // PINGtimer
            // 
            PINGtimer.Enabled = true;
            PINGtimer.Interval = 12000;
            PINGtimer.Tick += PINGtimer_Tick;
            // 
            // looptimer
            // 
            looptimer.Tick += looptimer_Tick;
            // 
            // STOPP_Timer
            // 
            STOPP_Timer.Interval = 5000;
            STOPP_Timer.Tick += STOPP_Timer_Tick;
            // 
            // labelCurrent0
            // 
            labelCurrent0.Location = new Point(7, 7);
            labelCurrent0.Name = "labelCurrent0";
            labelCurrent0.Size = new Size(85, 23);
            labelCurrent0.TabIndex = 21;
            // 
            // labelCurrent1
            // 
            labelCurrent1.Location = new Point(5, 5);
            labelCurrent1.Name = "labelCurrent1";
            labelCurrent1.Size = new Size(91, 23);
            labelCurrent1.TabIndex = 22;
            // 
            // panelCurrent0
            // 
            panelCurrent0.Controls.Add(labelCurrent0);
            panelCurrent0.Location = new Point(500, 186);
            panelCurrent0.Name = "panelCurrent0";
            panelCurrent0.Size = new Size(100, 35);
            panelCurrent0.TabIndex = 23;
            // 
            // panelCurrent1
            // 
            panelCurrent1.Controls.Add(labelCurrent1);
            panelCurrent1.Location = new Point(500, 226);
            panelCurrent1.Name = "panelCurrent1";
            panelCurrent1.Size = new Size(100, 35);
            panelCurrent1.TabIndex = 24;
            // 
            // cntPower
            // 
            cntPower.Location = new Point(499, 286);
            cntPower.Name = "cntPower";
            cntPower.Size = new Size(100, 23);
            cntPower.TabIndex = 25;
            // 
            // cntSwitch
            // 
            cntSwitch.Location = new Point(499, 332);
            cntSwitch.Name = "cntSwitch";
            cntSwitch.Size = new Size(100, 23);
            cntSwitch.TabIndex = 26;
            // 
            // cntSignal
            // 
            cntSignal.Location = new Point(499, 378);
            cntSignal.Name = "cntSignal";
            cntSignal.Size = new Size(100, 23);
            cntSignal.TabIndex = 27;
            // 
            // cntFeedback
            // 
            cntFeedback.Location = new Point(499, 424);
            cntFeedback.Name = "cntFeedback";
            cntFeedback.Size = new Size(100, 23);
            cntFeedback.TabIndex = 28;
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Location = new Point(499, 267);
            label4.Name = "label4";
            label4.Size = new Size(42, 15);
            label4.TabIndex = 29;
            label4.Text = "Strom:";
            // 
            // label5
            // 
            label5.AutoSize = true;
            label5.Location = new Point(499, 313);
            label5.Name = "label5";
            label5.Size = new Size(56, 15);
            label5.TabIndex = 30;
            label5.Text = "Weichen:";
            // 
            // label6
            // 
            label6.AutoSize = true;
            label6.Location = new Point(499, 359);
            label6.Name = "label6";
            label6.Size = new Size(48, 15);
            label6.TabIndex = 31;
            label6.Text = "Signale:";
            // 
            // label7
            // 
            label7.AutoSize = true;
            label7.Location = new Point(499, 405);
            label7.Name = "label7";
            label7.Size = new Size(73, 15);
            label7.TabIndex = 32;
            label7.Text = "Rückmelder:";
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = Color.FromArgb(245, 245, 245);
            ClientSize = new Size(604, 679);
            Controls.Add(label7);
            Controls.Add(label6);
            Controls.Add(label5);
            Controls.Add(label4);
            Controls.Add(cntFeedback);
            Controls.Add(cntSignal);
            Controls.Add(cntSwitch);
            Controls.Add(cntPower);
            Controls.Add(panelCurrent1);
            Controls.Add(panelCurrent0);
            Controls.Add(headerPanel);
            Controls.Add(listBoxLoks);
            Controls.Add(txtBridgeIP);
            Controls.Add(groupBox2);
            Controls.Add(tbMessages);
            Controls.Add(label3);
            Controls.Add(groupBox3);
            Controls.Add(lblBRIDGEStatus);
            Controls.Add(btnCopy);
            Controls.Add(tabControl1);
            Controls.Add(btnExit);
            Name = "Form1";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "CANguru-Server 1.0";
            Load += Form1_Load;
            headerPanel.ResumeLayout(false);
            headerPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)pictureBox1).EndInit();
            tabPage1.ResumeLayout(false);
            tabPage1.PerformLayout();
            tabPage2.ResumeLayout(false);
            tabPage2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)UpDnnextLokAddress).EndInit();
            ((System.ComponentModel.ISupportInitialize)UpDnnewSubscriptionCounter).EndInit();
            tabMain.ResumeLayout(false);
            tabMain.PerformLayout();
            groupBox3.ResumeLayout(false);
            tabControl1.ResumeLayout(false);
            panelCurrent0.ResumeLayout(false);
            panelCurrent0.PerformLayout();
            panelCurrent1.ResumeLayout(false);
            panelCurrent1.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private TabPage tabPage1;
        private TabPage tabPage2;
        private TabPage tabMain;
        private Button btnExit;
        private GroupBox groupBox2;
        private GroupBox groupBox3;
        private TabControl tabControl1;
        private TextBox txtBridgeIP;
        private TextBox listBoxCAN;
        private Button btnConnect;
        private Label lblBRIDGEStatus;
        private Label label3;
        private System.Windows.Forms.Timer canTimer;
        private System.Windows.Forms.Timer aliveTimer;
        private Button btnCopy;
        private System.Windows.Forms.Timer PINGtimer;
        private System.Windows.Forms.Timer looptimer;
        private TextBox tbMessages;
        private ListBox listBoxLoks;
        private Button btnCANguruLoks;
        private Label label2;
        private Label label1;
        private NumericUpDown UpDnnextLokAddress;
        private NumericUpDown UpDnnewSubscriptionCounter;
        private System.Windows.Forms.Timer STOPP_Timer;
        private Button btnLokListe;
        private Button btnDelete;
        private Button btnAddManualLok;
        private ListBox listBoxDecoder;
        private Button btnOtaUpdate;
        private RadioButton radioType1;
        private RadioButton radioType2;
        private RadioButton radioType3;
        private RadioButton radioType4;
        private RadioButton radioSingle;
        private TextBox labelCurrent0;
        private TextBox labelCurrent1;
        private Panel panelCurrent0;
        private Panel panelCurrent1;
        private Button buttonSettings;
        private Button btnSTOPP;
        private TextBox listBoxMFX;
        private PictureBox pictureBox1;
        private TextBox cntPower;
        private TextBox cntSwitch;
        private TextBox cntSignal;
        private TextBox cntFeedback;
        private Label label4;
        private Label label5;
        private Label label6;
        private Label label7;
        private Button showLokList;
        private Button showDecoderList;
    }
}
