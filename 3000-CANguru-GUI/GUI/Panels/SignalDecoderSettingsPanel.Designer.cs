using System.Xml.Linq;

/*
+-----------------------------------------------------------+
|  Label   Eingabe                                          |
|  Label   Eingabe                                          |
|  Label   Eingabe                                          |
|  Label   Eingabe                                          |
|  Label   Eingabe                                          |
+-----------------------------------------------------------+
|  +----------------------+   +---------------------------+ |
|  |      GroupBox       |   |       PictureBox          | |
|  |  Buttons untereinander| |   Stepper‑Bild rechts     | |
|  +----------------------+   +---------------------------+ |
+-----------------------------------------------------------+
*/
namespace CANguru.GUI.Panels
{
    partial class SignalDecoderSettingsPanel
    {
        private System.ComponentModel.IContainer components = null;

        public Label lblAddress;
        public Label lblDelay;
        public Label lblSteps;
        public Label lblRightLeft;
        public Label lblStepDir;

        public NumericUpDown numAddress;
        public NumericUpDown numDelay;
        public NumericUpDown numStepsToEnd;

        public ComboBox cmbRightLeft;
        public ComboBox cmbStepDirection;

        public PictureBox pictureBox;

        public Button btnSlowMove2Nullpkt;
        public Button btnSlowMove2End;
        public Button btnSetZero;
        public Button btnSetEnd;
        public Button btnTest;

        private GroupBox grpActions;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
                components.Dispose();
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            lblAddress = new Label();
            lblDelay = new Label();
            lblSteps = new Label();
            lblRightLeft = new Label();
            lblStepDir = new Label();
            numAddress = new NumericUpDown();
            numDelay = new NumericUpDown();
            numStepsToEnd = new NumericUpDown();
            cmbRightLeft = new ComboBox();
            cmbStepDirection = new ComboBox();
            pictureBox = new PictureBox();
            grpActions = new GroupBox();
            btnSlowMove2Nullpkt = new Button();
            btnSetZero = new Button();
            btnSlowMove2End = new Button();
            btnSetEnd = new Button();
            btnTest = new Button();
            ((System.ComponentModel.ISupportInitialize)numAddress).BeginInit();
            ((System.ComponentModel.ISupportInitialize)numDelay).BeginInit();
            ((System.ComponentModel.ISupportInitialize)numStepsToEnd).BeginInit();
            ((System.ComponentModel.ISupportInitialize)pictureBox).BeginInit();
            grpActions.SuspendLayout();
            SuspendLayout();
            // 
            // lblAddress
            // 
            lblAddress.AutoSize = true;
            lblAddress.Location = new Point(10, 10);
            lblAddress.Name = "lblAddress";
            lblAddress.Size = new Size(95, 15);
            lblAddress.TabIndex = 0;
            lblAddress.Text = "Weichenadresse:";
            // 
            // lblDelay
            // 
            lblDelay.AutoSize = true;
            lblDelay.Location = new Point(10, 40);
            lblDelay.Name = "lblDelay";
            lblDelay.Size = new Size(103, 15);
            lblDelay.TabIndex = 2;
            lblDelay.Text = "Verzögerung (ms):";
            // 
            // lblSteps
            // 
            lblSteps.AutoSize = true;
            lblSteps.Location = new Point(10, 70);
            lblSteps.Name = "lblSteps";
            lblSteps.Size = new Size(89, 15);
            lblSteps.TabIndex = 4;
            lblSteps.Text = "Gesamtschritte:";
            // 
            // lblRightLeft
            // 
            lblRightLeft.AutoSize = true;
            lblRightLeft.Location = new Point(10, 100);
            lblRightLeft.Name = "lblRightLeft";
            lblRightLeft.Size = new Size(121, 15);
            lblRightLeft.TabIndex = 6;
            lblRightLeft.Text = "Momentane Position:";
            // 
            // lblStepDir
            // 
            lblStepDir.AutoSize = true;
            lblStepDir.Location = new Point(10, 130);
            lblStepDir.Name = "lblStepDir";
            lblStepDir.Size = new Size(78, 15);
            lblStepDir.TabIndex = 8;
            lblStepDir.Text = "Laufrichtung:";
            // 
            // numAddress
            // 
            numAddress.Location = new Point(170, 8);
            numAddress.Name = "numAddress";
            numAddress.Size = new Size(120, 23);
            numAddress.TabIndex = 1;
            // 
            // numDelay
            // 
            numDelay.Location = new Point(170, 38);
            numDelay.Name = "numDelay";
            numDelay.Size = new Size(120, 23);
            numDelay.TabIndex = 3;
            // 
            // numStepsToEnd
            // 
            numStepsToEnd.Location = new Point(170, 68);
            numStepsToEnd.Name = "numStepsToEnd";
            numStepsToEnd.Size = new Size(120, 23);
            numStepsToEnd.TabIndex = 5;
            // 
            // cmbRightLeft
            // 
            cmbRightLeft.Location = new Point(170, 98);
            cmbRightLeft.Name = "cmbRightLeft";
            cmbRightLeft.Size = new Size(140, 23);
            cmbRightLeft.TabIndex = 7;
            // 
            // cmbStepDirection
            // 
            cmbStepDirection.Location = new Point(170, 128);
            cmbStepDirection.Name = "cmbStepDirection";
            cmbStepDirection.Size = new Size(140, 23);
            cmbStepDirection.TabIndex = 9;
            // 
            // pictureBox
            // 
            pictureBox.Image = Properties.Resources.Stepper;
            pictureBox.Location = new Point(178, 25);
            pictureBox.Name = "pictureBox";
            pictureBox.Size = new Size(196, 149);
            pictureBox.SizeMode = PictureBoxSizeMode.Zoom;
            pictureBox.TabIndex = 11;
            pictureBox.TabStop = false;
            // 
            // grpActions
            // 
            grpActions.Controls.Add(btnSlowMove2Nullpkt);
            grpActions.Controls.Add(btnSetZero);
            grpActions.Controls.Add(btnSlowMove2End);
            grpActions.Controls.Add(btnSetEnd);
            grpActions.Controls.Add(btnTest);
            grpActions.Controls.Add(pictureBox);
            grpActions.Location = new Point(10, 200);
            grpActions.Name = "grpActions";
            grpActions.Size = new Size(385, 180);
            grpActions.TabIndex = 12;
            grpActions.TabStop = false;
            grpActions.Text = "Einstellungen";
            // 
            // btnSlowMove2Nullpkt
            // 
            btnSlowMove2Nullpkt.Location = new Point(9, 25);
            btnSlowMove2Nullpkt.Name = "btnSlowMove2Nullpkt";
            btnSlowMove2Nullpkt.Size = new Size(159, 25);
            btnSlowMove2Nullpkt.TabIndex = 0;
            btnSlowMove2Nullpkt.Text = "Langsam zur Nullposition";
            // 
            // btnSetZero
            // 
            btnSetZero.Location = new Point(9, 55);
            btnSetZero.Name = "btnSetZero";
            btnSetZero.Size = new Size(159, 25);
            btnSetZero.TabIndex = 1;
            btnSetZero.Text = "Nullpunkt festlegen";
            // 
            // btnSlowMove2End
            // 
            btnSlowMove2End.Location = new Point(9, 85);
            btnSlowMove2End.Name = "btnSlowMove2End";
            btnSlowMove2End.Size = new Size(159, 25);
            btnSlowMove2End.TabIndex = 2;
            btnSlowMove2End.Text = "Langsam zur Endposition";
            // 
            // btnSetEnd
            // 
            btnSetEnd.Location = new Point(9, 115);
            btnSetEnd.Name = "btnSetEnd";
            btnSetEnd.Size = new Size(159, 25);
            btnSetEnd.TabIndex = 3;
            btnSetEnd.Text = "Endpunkt festlegen";
            // 
            // btnTest
            // 
            btnTest.Location = new Point(9, 145);
            btnTest.Name = "btnTest";
            btnTest.Size = new Size(159, 25);
            btnTest.TabIndex = 4;
            btnTest.Text = "Testfahrt";
            // 
            // SwitchDecoderSettingsPanel
            // 
            Controls.Add(lblAddress);
            Controls.Add(numAddress);
            Controls.Add(lblDelay);
            Controls.Add(numDelay);
            Controls.Add(lblSteps);
            Controls.Add(numStepsToEnd);
            Controls.Add(lblRightLeft);
            Controls.Add(cmbRightLeft);
            Controls.Add(lblStepDir);
            Controls.Add(cmbStepDirection);
            Controls.Add(grpActions);
            Name = "SwitchDecoderSettingsPanel";
            Size = new Size(402, 392);
            ((System.ComponentModel.ISupportInitialize)numAddress).EndInit();
            ((System.ComponentModel.ISupportInitialize)numDelay).EndInit();
            ((System.ComponentModel.ISupportInitialize)numStepsToEnd).EndInit();
            ((System.ComponentModel.ISupportInitialize)pictureBox).EndInit();
            grpActions.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }
    }
}
