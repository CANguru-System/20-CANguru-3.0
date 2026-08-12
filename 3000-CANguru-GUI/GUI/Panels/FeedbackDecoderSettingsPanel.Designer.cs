using System;
using System.Windows.Forms;

namespace CANguru.GUI.Panels
{
    partial class FeedbackDecoderSettingsPanel
    {
        private System.ComponentModel.IContainer components = null;

        private System.Windows.Forms.Label[] lblK;
        private System.Windows.Forms.NumericUpDown[] numK;

        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();

            lblK = new Label[16];
            numK = new NumericUpDown[16];

            int y = 10;

            for (int i = 0; i < 16; i++)
            {
                // Label erzeugen
                lblK[i] = new Label();
                lblK[i].Name = $"lblK{i + 1}";
                lblK[i].Text = $"Kanal {i + 1}";
                lblK[i].Left = 10;
                lblK[i].Top = y + 4;
                lblK[i].Width = 80;

                // NumericUpDown erzeugen
                numK[i] = new NumericUpDown();
                numK[i].Name = $"numK{i + 1}";
                numK[i].Left = 100;
                numK[i].Top = y;
                numK[i].Width = 60;
                numK[i].Minimum = 0;
                numK[i].Maximum = 255;

                // Controls hinzufügen
                this.Controls.Add(lblK[i]);
                this.Controls.Add(numK[i]);

                y += 30;
            }

            this.Width = 200;
            this.Height = 16 * 30 + 20;
        }
    }
}
