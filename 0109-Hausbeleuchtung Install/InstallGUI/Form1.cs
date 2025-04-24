using System;
using System.Management;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO.Ports;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using System.Threading;
using static System.Net.WebRequestMethods;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Button;
using System.Collections;
using System.Xml;
using System.Timers;
using System.Net;
using static System.Net.Mime.MediaTypeNames;
using QRCoder;
using System.Windows.Controls;

namespace InstallGUI
{
    public partial class Form1 : Form
    {
        // ************************ GLOBAL VARIABLES **************************************************************

        byte decoderCnt = 8;
        string credFile = "credentials.txt";
        const string esptool = "esptool.exe";
        SerialPort _serialPort;
        int count = 0;
        char content;
        bool bports;
        bool bssid;
        bool bpwd;
        enum decoders
        {
            gleisbesetztmelder = 0,
            stepper,
            bridge,
            booster,
            maxi,
            formsignal,
            next
        }
        struct decoderStruct
        {
            public String firmware_source;
            public String scanner_files;
            public String strprocessor;
            public bool credentials;
        }
        decoderStruct currDecoder;


        // delegate is used to write to a UI control from a non-UI thread
        private delegate void SetTextDeleg(string text);
        enum firmware
        {
            scanner,
            decoder,
            none
        }
        firmware loaded;
        private static System.Timers.Timer a1secTimer;
        ArrayList portlist = new ArrayList();
        UInt16 portlistPtr;

        public Form1()
        {
            InitializeComponent();

            //Subscribe to Event
            eyeicon.MouseDown += new MouseEventHandler(eyeicon_MouseDown);
            eyeicon.MouseUp += new MouseEventHandler(eyeicon_MouseUp);            // 

            password.UseSystemPasswordChar = true;
        }

        private void eyeicon_MouseDown(object sender, MouseEventArgs e)
        {
            password.UseSystemPasswordChar = false;

        }

        private void eyeicon_MouseUp(object sender, MouseEventArgs e)
        {
            password.UseSystemPasswordChar = true;

        }
        // ************************ FORM LOAD **************************************************************

        private void Form1_Load(object sender, EventArgs e)
        {
            bports = false;
            bssid = false;
            bpwd = false;
            loaded = firmware.none;
            String line;
            // weichenstepper
            currDecoder.firmware_source = "..\\0104-Weiche-Stepper-ESP32C3\\.pio\\build\\seeed_xiao_esp32c3\\";
            currDecoder.scanner_files = "ScanPorts\\.pio\\build\\seeed_xiao_esp32c3";
            currDecoder.strprocessor = "esp32c3";
            currDecoder.credentials = true;
            if (System.IO.File.Exists(credFile))
            {
                try
                {
                    //Pass the file path and file name to the StreamReader constructor
                    StreamReader sr = new StreamReader(credFile);
                    //Read the first line of text: COM-Port
                    line = sr.ReadLine();
                    lightportsBox.Items.Add(line);
                    lightportsBox.SelectedIndex = 0;
                    //        bports = true;
                    //Read the second line of text: SSID
                    line = sr.ReadLine();
                    ssidBox.Items.Add(line);
                    ssidBox.SelectedIndex = 0;
                    bssid = true;
                    //Read the third line of text: Password
                    line = sr.ReadLine();
                    password.Text = line;
                    //close the file
                    sr.Close();
                    reportBox.Text = "Gespeicherte Credentials eingelesen." + Environment.NewLine + "Drücken Sie den Knopf Upload, um die Firmware auf den ESP32 zu laden.";
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Exception: " + ex.Message);
                }
                finally
                {
                    Console.WriteLine("Executing finally block.");
                }
            }
            //
            _serialPort = new SerialPort();
            // Set the baud rate (e.g., 9600)
            _serialPort.BaudRate = 115200;
            // Set the parity (e.g., None)
            _serialPort.Parity = Parity.None;
            // Set the data bits (e.g., 8)
            _serialPort.DataBits = 8;
            // Set the stop bits (e.g., One)
            _serialPort.StopBits = StopBits.One;
            // Set the handshake (e.g., None)
            _serialPort.Handshake = Handshake.None;
            _serialPort.DataReceived += new SerialDataReceivedEventHandler(sp_DataReceived);
            _serialPort.ReadTimeout = 500;
            _serialPort.WriteTimeout = 500;
            // USB-Ports-Liste füllen
            scanPortsProc();
            // Decoderliste füllen
            for (byte d = 0; d < decoderCnt; d++)
            {
                decoderBox.Items.Add(d.ToString());
            }

            // Grenzen für onTime
            onTime.Maximum = 255;
            onTime.Minimum = 1;
            // Grenzen für offTime
            offTime.Maximum = 255;
            offTime.Minimum = 1;
            // Grenzen für brightness
            brightness.Maximum = 255;
            brightness.Minimum = 1;
            // Grenzen für baseTime
            baseTime.Maximum = 5000;
            baseTime.Minimum = 100;
            baseTime.Increment = 100;
        }

        // ************************ SAVE SSID AND PASSWORD **************************************************************

        private void saveBtn_Click(object sender, EventArgs e)
        {
            bpwd = password.Text.Length > 0;
            if (bports && bssid && bpwd)
            {
                String ssid = ssidBox.SelectedItem.ToString();
                string[] ssids = ssid.Split('(');
                try
                {
                    //Open the File
                    StreamWriter sw = new StreamWriter(credFile, false, Encoding.ASCII);

                    //Write out the port
                    sw.Write(lightportsBox.SelectedItem.ToString());
                    sw.Write("\n");
                    //Write out the ssid
                    sw.Write(ssids[0].Trim());
                    sw.Write("\n");
                    //Write out the password
                    sw.Write(password.Text);

                    //close the file
                    sw.Close();
                    reportBox.Text = "Credentials gespeichert.";
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Exception: " + ex.Message);
                }
                finally
                {
                    Console.WriteLine("Executing finally block.");
                }
            }
            else
                MessageBox.Show("Bitte zunächst den Port wählen, die SSID scannen und das Passwort eingaben!");
        }

        // ************************ FINISH APP **************************************************************

        private void finishBtn_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        // ************************ HELPER LOAD FIRMWARE TO DECODER **************************************************************

        bool loadFirmware(string cpu, string source, string txt, firmware fw)
        {
            int errNo = -1;
            reportBox.Text = "Bitte warten ...";
            try
            {
                if (_serialPort.IsOpen)
                    _serialPort.Close();

                Process P = new Process();
                P.StartInfo.FileName = esptool;
                // hier kann z.B. eine Textdatei mit übergeben werden
                if (cpu == "esp32")
                    P.StartInfo.Arguments = "--chip " + cpu + " --port " + lightportsBox.SelectedItem.ToString() + " --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 " + source + "/bootloader.bin 0x8000 " + source + "/partitions.bin 0x10000 " + source + "/firmware.bin";
                if (cpu == "esp32c3")
                    P.StartInfo.Arguments = "--chip " + cpu + " --port " + lightportsBox.SelectedItem.ToString() + " --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0000 " + source + "/bootloader.bin 0x8000 " + source + "/partitions.bin 0xE000 boot_app0.bin 0x10000 " + source + "/firmware.bin";
                processBox.Text = P.StartInfo.FileName + " " + P.StartInfo.Arguments + Environment.NewLine;
                P.StartInfo.UseShellExecute = false;
                P.StartInfo.RedirectStandardOutput = true;
                P.StartInfo.CreateNoWindow = true;
                P.Start();
                while (!P.StandardOutput.EndOfStream)
                {
                    processBox.AppendText(P.StandardOutput.ReadLine() + Environment.NewLine);
                }
                P.WaitForExit();
                errNo = P.ExitCode;
                P.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error ESPTOOL :: " + ex.Message, "Error!");
            }
            finally
            {
                if (errNo == 0)
                    reportBox.Text = txt + " installiert";
                else
                    reportBox.Text = txt + " nicht installiert! Fehler: " + errNo.ToString();
                loaded = fw;
            }
            return (errNo == 0);
        }

        // ************************ HELPER ERASE FLASH **************************************************************

        private void erasebtn_Click(object sender, EventArgs e)
        {
            int errNo = -1;
            reportBox.Text = "Bitte warten ...";
            try
            {
                Process P = new Process();
                P.StartInfo.FileName = esptool;
                P.StartInfo.Arguments = "--chip " + currDecoder.strprocessor + " --port " + lightportsBox.SelectedItem.ToString() + " erase_flash";
                processBox.Text = P.StartInfo.FileName + " " + P.StartInfo.Arguments + Environment.NewLine;
                P.StartInfo.UseShellExecute = false;
                P.StartInfo.RedirectStandardOutput = true;
                P.StartInfo.CreateNoWindow = true;
                P.Start();
                while (!P.StandardOutput.EndOfStream)
                {
                    processBox.AppendText(P.StandardOutput.ReadLine() + Environment.NewLine);
                }
                P.WaitForExit();
                errNo = P.ExitCode;
                P.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error ESPTOOL :: " + ex.Message, "Error!");
            }
            finally
            {
                if (errNo == 0)
                    reportBox.Text = "Flash-Speicher gelöscht";
                else
                    reportBox.Text = "Flash-Speicher nicht gelöscht! Fehler: " + errNo.ToString();
                loaded = firmware.none;
            }
        }

        // ************************ SERIAL COMMUNICATION **************************************************************

        /*       void openPort()
               {
                   try
                   {
                       if (lightportsBox.SelectedIndex == -1)
                       {
                           MessageBox.Show("Bitte zunächst ComPort festlegen", "Error!");
                           return;
                       }
                       if (_serialPort == null)
                       {
                           // noch kein serial-Port
                           _serialPort = new SerialPort(lightportsBox.SelectedItem.ToString(), 115200, Parity.None, 8, StopBits.One);
                           _serialPort.Handshake = Handshake.None;
                           _serialPort.DataReceived += new SerialDataReceivedEventHandler(sp_DataReceived);
                           _serialPort.ReadTimeout = 500;
                           _serialPort.WriteTimeout = 500;
                           _serialPort.Open();
                           return;
                       }
                       // ist geöffnet und korrekter Port: keine Aktion
                       if ((_serialPort.IsOpen) && (_serialPort.PortName == lightportsBox.SelectedItem.ToString()))
                           return;
                       else
                       // ist geöffnet oder falscher Port: Schließen und neu
                       if ((_serialPort.IsOpen) || (_serialPort.PortName != lightportsBox.SelectedItem.ToString()))
                           _serialPort.Close();
                       openPort();

                   }
                   catch (Exception ex)
                   {
                       MessageBox.Show("Error opening to serial port :: " + ex.Message, "Error!");
                   }
               }
       */
        bool write2Port(string subcommand)
        {
            //
            // Makes sure serial port is open before trying to write
            // all of the options for a serial device
            // can be sent through the constructor of the SerialPort class
            // PortName = "COM1", Baud Rate = 19200, Parity = None, 
            // Data Bits = 8, Stop Bits = One, Handshake = None

            try
            {
                if (!_serialPort.IsOpen)
                    _serialPort.Open();

                _serialPort.Write(subcommand);

                //  if (lightportsBox.SelectedIndex != -1)
                //        this.processBox.Invoke(new MethodInvoker(() => this.processBox.AppendText("CMD:" + lightportsBox.SelectedItem.ToString() + ": " + subcommand + Environment.NewLine)));
                //    processBox.AppendText("CMD:" + lightportsBox.SelectedItem.ToString() + ": " + subcommand + Environment.NewLine);
                return true;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error opening/writing to serial port :: " + ex.Message, "Error!");
                return false;
            }
        }

        void sp_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            Thread.Sleep(100);
            string data = _serialPort.ReadLine();
            this.BeginInvoke(new SetTextDeleg(si_DataReceived), new object[] { data });
        }

        private void si_DataReceived(string data)
        {
            string[] loads;
            processBox.AppendText(data + " gefunden" + Environment.NewLine);
            if (data.IndexOf('&') == 0)
            {
                count = data[1] - 0x30;
                content = data[2];
            }
            else
            {
                string[] charsToRemove = new string[] { "\n", "\r" };
                foreach (var c in charsToRemove)
                {
                    data = data.Replace(c, string.Empty);
                }
                switch (content)
                {
                    case 'A':
                        loads = data.Split('!');
                        onTime.Value = Decimal.Parse(loads[0]);
                        offTime.Value = Decimal.Parse(loads[1]);
                        brightness.Value = Decimal.Parse(loads[2]);
                        baseTime.Value = Decimal.Parse(loads[3]);
                        reportBox.Text = "Daten geladen.";
                        break;
                    case 'B':
                        processBox.AppendText((string)portlist[portlistPtr - 1] + " gefunden" + Environment.NewLine);
                        lightportsBox.Items.Add((string)portlist[portlistPtr - 1]);
                        if (portlist.Count == portlistPtr)
                        {
                            // end of list
                            if (lightportsBox.Items.Count > 0)
                            {
                                lightportsBox.SelectedIndex = 0;
                                bports = true;
                                reportBox.Text = "COM-Ports geladen.";
                                string command = "DEC" + decoderBox.GetItemText(decoderBox.SelectedItem);
                                bssid = write2Port(command);
                            }
                            else
                            {
                                reportBox.Text = "KEINE COM-Ports gefunden.";
                                lightportsBox.Items.Add("kein Port!");
                            }
                        }
                        break;
                    case 'C':
                        // L!index!ontime!offTime!brightness
                        // L!3!27!39!32
                        // cnt = 12;
                        String v0 = decoderBox.SelectedIndex.ToString();
                        String v1 = onTime.Value.ToString();
                        String v2 = offTime.Value.ToString();
                        String v3 = brightness.Value.ToString();
                        String v4 = baseTime.Value.ToString();
                        String v = "L!" + v0 + '!' + v1 + '!' + v2 + '!' + v3 + '!' + v4;
                        bssid = write2Port(v);
                        break;
                    default:
                        break;
                }
            }
        }

        // ************************ SCAN SSID **************************************************************
        private void scanSSIDBtn_Click(object sender, EventArgs e)
        {
            // Clear the listbox
            ssidBox.Items.Clear();
            if (loadFirmware(currDecoder.strprocessor, currDecoder.scanner_files, "Scanfirmware", firmware.scanner))
                bssid = write2Port("SCAN");
            //
        }

        // ************************ SCAN PORTS **************************************************************

        private void Seta1secTimer()
        {
            // Create a timer with a one second interval.
            a1secTimer = new System.Timers.Timer(1000);
            // Hook up the Elapsed event for the timer. 
            a1secTimer.Elapsed += testPort;
            a1secTimer.AutoReset = true;
            a1secTimer.Enabled = true;
        }

        private void testPort(Object source, ElapsedEventArgs e)
        {
            string cmd = "USB0";
            changePortname((string)portlist[portlistPtr]);
            this.processBox.Invoke(new MethodInvoker(() => this.processBox.AppendText("testPort: " + (string)portlist[portlistPtr] + " - " + cmd + Environment.NewLine)));
            write2Port(cmd);
            portlistPtr++;
            if (portlist.Count == portlistPtr)
            {
                a1secTimer.Enabled = false;
                //    this.decoderBox.Invoke(new MethodInvoker(() => this.decoderBox.SelectedIndex = 0));
            }
        }

        private void scanPortsProc()
        {
            // Clear the listboxes
            lightportsBox.Items.Clear();
            comportsBox.Items.Clear();

            portlist.Clear();

            // Get a list of serial port names.
            string[] ports = SerialPort.GetPortNames();

            foreach (string port in ports)
            {
                comportsBox.Items.Add(port);
                portlist.Add(port);
                processBox.AppendText(port + Environment.NewLine);
            }
            if (comportsBox.Items.Count > 0)
            {
                comportsBox.SelectedIndex = 0;
                bports = true;
                reportBox.Text = "COM-Ports geladen.";
            }
            else
            {
                reportBox.Text = "KEINE COM-Ports gefunden.";
                comportsBox.Items.Add("kein Port!");
            }

            if (portlist.Count > 0)
            {
                portlistPtr = 0;
                Seta1secTimer();
            }
        }


        private void scanPortsBtn_Click(object sender, EventArgs e)
        {
            // Clear the listbox
            comportsBox.Items.Clear();
            // Get a list of serial port names.
            string[] ports = SerialPort.GetPortNames();

            foreach (string port in ports)
            {
                comportsBox.Items.Add(port);
            }
            if (comportsBox.Items.Count > 0)
            {
                comportsBox.SelectedIndex = 0;
                bports = true;
                reportBox.Text = "COM-Ports geladen.";
            }
            else
            {
                reportBox.Text = "KEINE COM-Ports gefunden.";
                comportsBox.Items.Add("kein Port!");
            }
        }

        private void changePortname(string pn)
        {
            if (_serialPort.IsOpen)
                _serialPort.Close();
            // Set the port name (e.g., "COM3")
            _serialPort.PortName = pn;
            if (!_serialPort.IsOpen)
                _serialPort.Open();
        }

        private void lightportsBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (_serialPort == null)
            {
                return;
            }

            if (lightportsBox.SelectedItem.ToString() != _serialPort.PortName)
            {
                changePortname(lightportsBox.SelectedItem.ToString());
            }
        }

        // ************************ UPLOAD DECODER-FIRMWARE **************************************************************

        private void uploadBtn_Click(object sender, EventArgs e)
        {
            string command;
            bool res = true;
            if (currDecoder.credentials == true)
            {
                // load scanner firmware to decoder
                if (loaded != firmware.scanner)
                    res = loadFirmware(currDecoder.strprocessor, currDecoder.scanner_files, "Scanfirmware", firmware.scanner);
                // prepare and send ssid via port
                if (res && (ssidBox.Items.Count > 0))
                {
                    String ssid = ssidBox.SelectedItem.ToString();
                    if (ssidBox.SelectedItem.ToString().IndexOf('(') != -1)
                    {
                        string[] ssids = ssid.Split('(');
                        command = "SSID" + ssids[0].Trim();
                    }
                    else
                        command = "SSID" + ssid;
                    bssid = write2Port(command);
                }
                else
                    MessageBox.Show("FEHLER!");
            }
            else
                loadFirmware(currDecoder.strprocessor, currDecoder.firmware_source, "Decoderfirmware", firmware.decoder);
        }

        // ************************ HELP WINDOW **************************************************************

        private void helpbtn_Click(object sender, EventArgs e)
        {
            // create an object of `Form2` form in the current form
            Form2 helpForm = new Form2();

            // use the `Show()` method to access the new non-modal form
            helpForm.ShowDialog();
        }

        private void transmitBtn_Click(object sender, EventArgs e)
        {
            string command;
            // TXXX
            // L!index!ontime!offTime!brightness
            // L!3!27!39!32
            // cnt = 12;
            String v0 = decoderBox.SelectedIndex.ToString();
            String v1 = onTime.Value.ToString();
            String v2 = offTime.Value.ToString();
            String v3 = brightness.Value.ToString();
            String v4 = baseTime.Value.ToString();
            String v = "L!" + v0 + '!' + v1 + '!' + v2 + '!' + v3 + '!' + v4;
            int cnt = v.Length;
            String sCnt = cnt.ToString().PadLeft(3, '0');
            command = "T" + sCnt;
            bssid = write2Port(command);
        }

        private void decoderBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (bports)
            {
                string command = "DEC" + decoderBox.GetItemText(decoderBox.SelectedItem);
                bssid = write2Port(command);
            }
        }
    }
}
