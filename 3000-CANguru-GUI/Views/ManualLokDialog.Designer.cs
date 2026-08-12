namespace CANguru.Views
{
    partial class ManualLokDialog
    {
        private System.ComponentModel.IContainer components = null;

        private TextBox textName;
        private NumericUpDown numericSID;
        private ComboBox comboDecoderType;
        private Button buttonOK;
        private Button buttonCancel;
        private Label labelName;
        private Label labelSID;
        private Label labelDecoder;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
                components.Dispose();
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.textName = new TextBox();
            this.numericSID = new NumericUpDown();
            this.comboDecoderType = new ComboBox();
            this.buttonOK = new Button();
            this.buttonCancel = new Button();
            this.labelName = new Label();
            this.labelSID = new Label();
            this.labelDecoder = new Label();

            ((System.ComponentModel.ISupportInitialize)(this.numericSID)).BeginInit();
            this.SuspendLayout();

            // Name
            this.labelName.Text = "Name:";
            this.labelName.Location = new System.Drawing.Point(12, 15);
            this.textName.Location = new System.Drawing.Point(120, 12);
            this.textName.Width = 200;

            // SID
            this.labelSID.Text = "Adresse (SID):";
            this.labelSID.Location = new System.Drawing.Point(12, 55);
            this.numericSID.Location = new System.Drawing.Point(120, 52);
            this.numericSID.Minimum = 1;
            this.numericSID.Maximum = 255;

            // Decoder
            this.labelDecoder.Text = "Decoder-Typ:";
            this.labelDecoder.Location = new System.Drawing.Point(12, 95);
            this.comboDecoderType.Location = new System.Drawing.Point(120, 92);
            this.comboDecoderType.DropDownStyle = ComboBoxStyle.DropDownList;

            // Buttons
            this.buttonOK.Text = "OK";
            this.buttonOK.Location = new System.Drawing.Point(120, 140);
            this.buttonOK.Click += new EventHandler(this.buttonOK_Click);

            this.buttonCancel.Text = "Abbrechen";
            this.buttonCancel.Location = new System.Drawing.Point(210, 140);
            this.buttonCancel.Click += new EventHandler(this.buttonCancel_Click);

            // Form
            this.ClientSize = new System.Drawing.Size(340, 190);
            this.Controls.Add(this.labelName);
            this.Controls.Add(this.textName);
            this.Controls.Add(this.labelSID);
            this.Controls.Add(this.numericSID);
            this.Controls.Add(this.labelDecoder);
            this.Controls.Add(this.comboDecoderType);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.StartPosition = FormStartPosition.CenterParent;
            this.Text = "Manuelle Lok hinzufügen";

            ((System.ComponentModel.ISupportInitialize)(this.numericSID)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
        }
    }
}