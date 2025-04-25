using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace InstallGUI
{
    public partial class Form2 : Form
    {
        private System.Windows.Forms.Button closebtn;
        private System.Windows.Forms.TextBox helptextbox;

        [DllImport("user32")]
        private static extern bool HideCaret(IntPtr hWnd);

        private void InitializeComponent()
        {
            this.helptextbox = new System.Windows.Forms.TextBox();
            this.closebtn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // helptextbox
            // 
            this.helptextbox.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.helptextbox.Location = new System.Drawing.Point(72, 51);
            this.helptextbox.Multiline = true;
            this.helptextbox.Name = "helptextbox";
            this.helptextbox.ReadOnly = true;
            this.helptextbox.Size = new System.Drawing.Size(434, 504);
            this.helptextbox.TabIndex = 0;
            // 
            // closebtn
            // 
            this.closebtn.Location = new System.Drawing.Point(234, 581);
            this.closebtn.Name = "closebtn";
            this.closebtn.Size = new System.Drawing.Size(75, 23);
            this.closebtn.TabIndex = 1;
            this.closebtn.Text = "Schliessen";
            this.closebtn.UseVisualStyleBackColor = true;
            this.closebtn.Click += new System.EventHandler(this.closebtn_Click);
            // 
            // Form2
            // 
            this.ClientSize = new System.Drawing.Size(565, 635);
            this.Controls.Add(this.closebtn);
            this.Controls.Add(this.helptextbox);
            this.Name = "Form2";
            this.Text = "Hilfe zum CANguru-Installationsprogramm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        public Form2()
        {
            InitializeComponent();
            helptextbox.Clear();
            helptextbox.Text = "Wenn Sie einen Decoder mit Software beladen wollen," +
                               "dann gehen Sie bitte in den folgenden Schritten vor:\r\n\r\n\r\n" +
            "1. Drücken Sie den Button SCAN, um die belegten Ports zu sehen und wählen Sie den relevanten aus.\r\n\r\n" +      // 
            "2. Nun drücken Sie Scan SSIDs. Anschließend werden Ihnen die Namen der verfügbaren Netze angezeigt. Wählen Sie den relevanten aus.\r\n\r\n" +       // 
            "3. Nun geben Sie das zugehörige Passwort ein.\r\n\r\n" +       // 
            "4. Nun wählen Sie den Decoder aus der Liste aus, den Sie bespielen möchten.\r\n\r\n" +        // 
            "5. Sie können nun den Button Upload drücken, um die Firmware des Decoders aufzuspielen.\r\n\r\n" +        // 
            "6. Mit dem Button Erase flash können Sie einen Decoder vollständig löschen.\r\n\r\n" +        // 
            "7. Mit dem Button Speichern können Sie den Port, den SSID und das Passwort sichern.\r\n\r\n";        // 
            helptextbox.SelectionStart = 0;
            helptextbox.SelectionLength = 0;
            helptextbox.Select();
            helptextbox.GotFocus += helptextbox_GotFocus;
            //   helptextbox.Cursor = Cursors.Arrow; //To disable the cursor
        }

        private void helptextbox_GotFocus(object sender, EventArgs e)
        {
            HideCaret(helptextbox.Handle);
        }
        private void closebtn_Click(object sender, System.EventArgs e)
        {
            this.Close();
        }
    }
}