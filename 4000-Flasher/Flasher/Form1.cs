using System;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;
using System.Windows.Forms;

namespace FirmwareFlasher
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            LoadPorts();
        }
        private string DetectChip(string port)
        {
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = "esptool.exe";
            psi.Arguments = $"--port {port} chip_id";
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.UseShellExecute = false;
            psi.CreateNoWindow = true;

            Process p = new Process();
            p.StartInfo = psi;

            p.Start();

            string output = p.StandardOutput.ReadToEnd();
            output += p.StandardError.ReadToEnd();

            p.WaitForExit();

            Log(output);

            if (output.Contains("ESP32-C3"))
                return "esp32c3";

            if (output.Contains("ESP32-S3"))
                return "esp32s3";

            if (output.Contains("ESP32-S2"))
                return "esp32s2";

            if (output.Contains("ESP32"))
                return "esp32";

            return "unknown";
        }

        private void LoadPorts()
        {
            cmbPorts.Items.Clear();
            cmbPorts.Items.AddRange(SerialPort.GetPortNames());
            if (cmbPorts.Items.Count > 0)
                cmbPorts.SelectedIndex = 0;
        }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "Firmware (*.bin)|*.bin";
            if (dlg.ShowDialog() == DialogResult.OK)
                txtFirmware.Text = dlg.FileName;
        }

        private void btnFlash_Click(object sender, EventArgs e)
        {
            if (!File.Exists(txtFirmware.Text))
            {
                Log("Bitte gültige Firmware auswählen.");
                return;
            }

            if (cmbPorts.SelectedItem == null)
            {
                Log("Bitte COM-Port auswählen.");
                return;
            }

            string port = cmbPorts.SelectedItem.ToString();
            string firmware = txtFirmware.Text;

            FlashFirmware(port, firmware);
        }

        private void FlashFirmware(string port, string firmware)
        {
            Log($"Erkenne Chip auf {port} ...");

            string chip = DetectChip(port);

            if (chip == "unknown")
            {
                Log("Konnte Chip nicht erkennen. Abbruch.");
                return;
            }

            Log($"Gefundener Chip: {chip}");

            string args = "";

            if (chip == "esp32")
            {
                args = $"--chip esp32 --port {port} --baud 460800 write_flash -z 0x10000 \"{firmware}\"";
            }
            else if (chip == "esp32c3")
            {
                args = $"--chip esp32c3 --port {port} --baud 460800 write_flash -z 0x10000 \"{firmware}\"";
            }
            else if (chip == "esp32s3")
            {
                args = $"--chip esp32s3 --port {port} --baud 460800 write_flash -z 0x0000 \"{firmware}\"";
            }
            else if (chip == "esp32s2")
            {
                args = $"--chip esp32s2 --port {port} --baud 460800 write_flash -z 0x0000 \"{firmware}\"";
            }

            Log("Starte Flash-Vorgang ...");

            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = "esptool.exe";
            psi.Arguments = args;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.UseShellExecute = false;
            psi.CreateNoWindow = true;

            Process p = new Process();
            p.StartInfo = psi;

            p.OutputDataReceived += (s, e) => { if (e.Data != null) Log(e.Data); };
            p.ErrorDataReceived += (s, e) => { if (e.Data != null) Log("ERR: " + e.Data); };

            p.Start();
            p.BeginOutputReadLine();
            p.BeginErrorReadLine();
        }

        private void Log(string msg)
        {
            txtLog.AppendText(msg + Environment.NewLine);
        }

        private void EraseFlash(string port)
        {
            Log($"Erkenne Chip auf {port} ...");

            string chip = DetectChip(port);

            if (chip == "unknown")
            {
                Log("Konnte Chip nicht erkennen. Abbruch.");
                return;
            }

            Log($"Gefundener Chip: {chip}");

            string args = "";

            if (chip == "esp32")
            {
                args = $"--chip esp32 --port {port} --baud 460800 erase_flash";
            }
            else if (chip == "esp32c3")
            {
                args = $"--chip esp32c3 --port {port} --baud 460800 erase_flash";
            }
            else if (chip == "esp32s3")
            {
                args = $"--chip esp32s3 --port {port} --baud 460800 erase_flash";
            }
            else if (chip == "esp32s2")
            {
                args = $"--chip esp32s2 --port {port} --baud 460800 erase_flash";
            }

        Log("Starte Flash-Vorgang ...");
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = "esptool.exe";
            psi.Arguments = args;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.UseShellExecute = false;
            psi.CreateNoWindow = true;

            Process p = new Process();
            p.StartInfo = psi;

            p.OutputDataReceived += (s, e) => { if (e.Data != null) Log(e.Data); };
            p.ErrorDataReceived += (s, e) => { if (e.Data != null) Log("ERR: " + e.Data); };

            p.Start();
            p.BeginOutputReadLine();
            p.BeginErrorReadLine();
        }

        private void btnEraseFlash_Click(object sender, EventArgs e)
        {

            if (cmbPorts.SelectedItem == null)
            {
                Log("Bitte COM-Port auswählen.");
                return;
            }

            string port = cmbPorts.SelectedItem.ToString();

            EraseFlash(port);
        }
    }
}
