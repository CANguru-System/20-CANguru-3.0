using System;
using System.Drawing;
using System.Windows.Forms;

namespace CANguru.Controls
{
    public class ToggleStopGoButton : Button
    {
        public bool IsGoMode { get; private set; } = false;

        // Events für deine Aktionen
        public event EventHandler StopClicked;
        public event EventHandler GoClicked;

        public ToggleStopGoButton()
        {
            this.Enabled = false; // ganz am Anfang NICHT anwählbar
            this.FlatStyle = FlatStyle.Flat;
            this.FlatAppearance.BorderSize = 1;
            this.Font = new Font("Segoe UI", 14, FontStyle.Bold);

            UpdateAppearance();

            this.Click += ToggleStopGoButton_Click;
        }

        private void ToggleStopGoButton_Click(object sender, EventArgs e)
        {
            // Zustand umschalten
            IsGoMode = !IsGoMode;
            UpdateAppearance();

            // Aktion auslösen
            if (IsGoMode)
                GoClicked?.Invoke(this, EventArgs.Empty);
            else
                StopClicked?.Invoke(this, EventArgs.Empty);
        }

        private void UpdateAppearance()
        {
            if (IsGoMode)
            {
                this.Text = "⛔ STOPP";
                this.BackColor = Color.Red;
                this.ForeColor = Color.White;
            }
            else
            {
                this.Text = "▶ GO";
                this.BackColor = Color.LimeGreen;
                this.ForeColor = Color.Black;
            }
        }

        // Optional: Zustand programmatisch setzen
        public void SetMode(bool goMode)
        {
            IsGoMode = goMode;
            UpdateAppearance();
        }
    }
}
