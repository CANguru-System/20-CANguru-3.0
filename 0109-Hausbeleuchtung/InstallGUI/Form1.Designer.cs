﻿using System.Windows.Forms;

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
            this.button2 = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.lightportsBox = new System.Windows.Forms.ListBox();
            this.reportBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.processBox = new System.Windows.Forms.TextBox();
            this.transmitBtn = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.decoderBox = new System.Windows.Forms.ComboBox();
            this.raeume = new System.Windows.Forms.GroupBox();
            this.resetBtn = new System.Windows.Forms.Button();
            this.baseTime = new System.Windows.Forms.NumericUpDown();
            this.brightness = new System.Windows.Forms.NumericUpDown();
            this.offTime = new System.Windows.Forms.NumericUpDown();
            this.onTime = new System.Windows.Forms.NumericUpDown();
            this.comportsBox = new System.Windows.Forms.ListBox();
            this.label11 = new System.Windows.Forms.Label();
            this.raeume.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.baseTime)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.brightness)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.offTime)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.onTime)).BeginInit();
            this.SuspendLayout();
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(380, 344);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 5;
            this.button2.Text = "Beenden";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.finishBtn_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(267, 155);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(102, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "Beleuchtungs-Ports:";
            // 
            // lightportsBox
            // 
            this.lightportsBox.FormattingEnabled = true;
            this.lightportsBox.Location = new System.Drawing.Point(270, 170);
            this.lightportsBox.Name = "lightportsBox";
            this.lightportsBox.Size = new System.Drawing.Size(185, 43);
            this.lightportsBox.TabIndex = 9;
            this.lightportsBox.SelectedIndexChanged += new System.EventHandler(this.lightportsBox_SelectedIndexChanged);
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
            this.transmitBtn.Location = new System.Drawing.Point(245, 25);
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
            this.label7.Location = new System.Drawing.Point(209, 338);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(48, 13);
            this.label7.TabIndex = 28;
            this.label7.Text = "Basiszeit";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(17, 303);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(42, 13);
            this.label8.TabIndex = 29;
            this.label8.Text = "onTime";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(17, 338);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(42, 13);
            this.label9.TabIndex = 30;
            this.label9.Text = "offTime";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(209, 307);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(50, 13);
            this.label10.TabIndex = 31;
            this.label10.Text = "Helligkeit";
            // 
            // decoderBox
            // 
            this.decoderBox.FormattingEnabled = true;
            this.decoderBox.Location = new System.Drawing.Point(165, 27);
            this.decoderBox.Name = "decoderBox";
            this.decoderBox.Size = new System.Drawing.Size(40, 21);
            this.decoderBox.TabIndex = 33;
            this.decoderBox.SelectedIndexChanged += new System.EventHandler(this.decoderBox_SelectedIndexChanged);
            // 
            // raeume
            // 
            this.raeume.Controls.Add(this.resetBtn);
            this.raeume.Controls.Add(this.baseTime);
            this.raeume.Controls.Add(this.brightness);
            this.raeume.Controls.Add(this.decoderBox);
            this.raeume.Controls.Add(this.offTime);
            this.raeume.Controls.Add(this.onTime);
            this.raeume.Controls.Add(this.transmitBtn);
            this.raeume.Location = new System.Drawing.Point(12, 237);
            this.raeume.Name = "raeume";
            this.raeume.Size = new System.Drawing.Size(332, 139);
            this.raeume.TabIndex = 35;
            this.raeume.TabStop = false;
            this.raeume.Text = "Räume";
            // 
            // resetBtn
            // 
            this.resetBtn.Location = new System.Drawing.Point(17, 27);
            this.resetBtn.Name = "resetBtn";
            this.resetBtn.Size = new System.Drawing.Size(75, 23);
            this.resetBtn.TabIndex = 34;
            this.resetBtn.Text = "Reset";
            this.resetBtn.UseVisualStyleBackColor = true;
            this.resetBtn.Click += new System.EventHandler(this.resetBtn_Click);
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
            this.ClientSize = new System.Drawing.Size(479, 387);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.comportsBox);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.reportBox);
            this.Controls.Add(this.processBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.lightportsBox);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.raeume);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "Hausbeleuchtung-Installationsprogramm 1.0";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.raeume.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.baseTime)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.brightness)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.offTime)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.onTime)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ListBox lightportsBox;
        private System.Windows.Forms.TextBox reportBox;
        private System.Windows.Forms.Label label6;
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
        private Button resetBtn;
    }
}

