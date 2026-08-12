using CANguru.GUI.Panels;
using CANguru.DecoderControls;
namespace CANguru.GUI.Forms
{
    partial class FeedbackDecoderSettingsForm
    {
        private System.ComponentModel.IContainer components = null;

        private FeedbackDecoderSettingsPanel feedbackPanel;
        private Button btnSave;
        private Button btnCancel;
        private Button btnReset;

        private void InitializeComponent()
        {
            feedbackPanel = new FeedbackDecoderSettingsPanel();
            btnSave = new Button();
            btnCancel = new Button();
            btnReset = new Button();
            SuspendLayout();
            // 
            // feedbackPanel
            // 
            feedbackPanel.Location = new Point(10, 10);
            feedbackPanel.Name = "feedbackPanel";
            feedbackPanel.Size = new Size(300, 500);
            feedbackPanel.TabIndex = 0;
            // 
            // btnSave
            // 
            btnSave.Location = new Point(122, 523);
            btnSave.Name = "btnSave";
            btnSave.Size = new Size(100, 23);
            btnSave.TabIndex = 1;
            btnSave.Text = "Speichern";
            btnSave.Click += btnSave_Click;
            // 
            // btnCancel
            // 
            btnCancel.Location = new Point(122, 565);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new Size(100, 23);
            btnCancel.TabIndex = 2;
            btnCancel.Text = "Schließen";
            btnCancel.Click += btnCancel_Click;
            // 
            // btnReset
            // 
            btnReset.Location = new Point(16, 523);
            btnReset.Name = "btnReset";
            btnReset.Size = new Size(100, 23);
            btnReset.TabIndex = 3;
            btnReset.Text = "Reset";
            btnReset.Click += btnReset_Click;
            // 
            // FeedbackDecoderSettingsForm
            // 
            ClientSize = new Size(248, 600);
            Controls.Add(feedbackPanel);
            Controls.Add(btnSave);
            Controls.Add(btnCancel);
            Controls.Add(btnReset);
            Name = "FeedbackDecoderSettingsForm";
            Text = "Feedback-Decoder Einstellungen";
            Load += FeedbackDecoderSettingsForm_Load;
            ResumeLayout(false);
        }
    }
}
