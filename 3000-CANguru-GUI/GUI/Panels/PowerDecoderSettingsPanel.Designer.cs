using System.Drawing;
using System.Windows.Forms;

namespace CANguru.GUI.Panels
{
    partial class PowerDecoderSettingsPanel
    {
        private System.ComponentModel.IContainer components = null;

        private Label lblThreshold;
        public NumericUpDown numThreshold;

        private void InitializeComponent()
        {
            lblThreshold = new Label();
            numThreshold = new NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)numThreshold).BeginInit();
            SuspendLayout();
            // 
            // lblThreshold
            // 
            lblThreshold.AutoSize = true;
            lblThreshold.Location = new Point(10, 15);
            lblThreshold.Name = "lblThreshold";
            lblThreshold.Size = new Size(103, 15);
            lblThreshold.TabIndex = 0;
            lblThreshold.Text = "Schwellwert (mA):";
            // 
            // numThreshold
            // 
            numThreshold.Increment = new decimal(new int[] { 50, 0, 0, 0 });
            numThreshold.Location = new Point(150, 13);
            numThreshold.Maximum = new decimal(new int[] { 5000, 0, 0, 0 });
            numThreshold.Minimum = new decimal(new int[] { 100, 0, 0, 0 });
            numThreshold.Name = "numThreshold";
            numThreshold.Size = new Size(120, 23);
            numThreshold.TabIndex = 1;
            numThreshold.Value = new decimal(new int[] { 100, 0, 0, 0 });
            // 
            // PowerDecoderSettingsPanel
            // 
            Controls.Add(lblThreshold);
            Controls.Add(numThreshold);
            Name = "PowerDecoderSettingsPanel";
            Size = new Size(325, 51);
            ((System.ComponentModel.ISupportInitialize)numThreshold).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }
    }
}
