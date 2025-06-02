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
using System.Windows.Documents;

namespace InstallGUI
{
    public partial class Form1 : Form
    {
        // ************************ GLOBAL VARIABLES **************************************************************

        byte decoderCnt = 8;
        SerialPort _serialPort;
        char content;
        bool bports;
        bool bssid;
        bool makeReceive;

        // delegate is used to write to a UI control from a non-UI thread
        private delegate void SetTextDeleg(string text);
        private static System.Timers.Timer ahalfsecTimer;
        ArrayList portlist = new ArrayList();
        UInt16 portlistPtr;

        public Form1()
        {
            InitializeComponent();
        }

        // ************************ FORM LOAD **************************************************************

        private void Form1_Load(object sender, EventArgs e)
        {
            bports = false;
            bssid = false;
            makeReceive = true;
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
            baseTime.Maximum = 100000;
            baseTime.Minimum = 100;
            baseTime.Increment = 100;
        }

        // ************************ FINISH APP **************************************************************

        private void finishBtn_Click(object sender, EventArgs e)
        {
            this.Close();
        }

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

                _serialPort.Write(subcommand + '$');

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
            string data = _serialPort.ReadLine();
            // makeReceive sperrt Datenverarbeitung beim Neustart des ESP
            // und blendet Meldungen des ESP aus
            if (makeReceive)
                this.BeginInvoke(new SetTextDeleg(si_DataReceived), new object[] { data });
            else
            {
                if (data.Contains("BACK"))
                {
                    makeReceive = true;
                    bssid = write2Port("DEC0");
                }
            }

        }

        private void si_DataReceived(string data)
        {
            string[] loads;
            if (data.IndexOf('&') == 0)
            {
                content = data[1];
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
                        processBox.AppendText((string)portlist[portlistPtr - 1] + " als Beleuchtungsport registriert" + Environment.NewLine);
                        reportBox.Text = (string)portlist[portlistPtr - 1] + " registriert";
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
                        processBox.AppendText("Daten gespeichert" + Environment.NewLine);
                        reportBox.Text = "Daten gespeichert.";
                        break;
                    default:
                        break;
                }
            }
        }

        // ************************ SCAN PORTS **************************************************************

        private void Seta1halfsecTimer()
        {
            // Create a timer with a one second interval.
            ahalfsecTimer = new System.Timers.Timer(500);
            // Hook up the Elapsed event for the timer. 
            ahalfsecTimer.Elapsed += testPort;
            ahalfsecTimer.AutoReset = true;
            ahalfsecTimer.Enabled = true;
        }

        private void testPort(Object source, ElapsedEventArgs e)
        {
            string cmd = "USB0";
            changePortname((string)portlist[portlistPtr]);
            write2Port(cmd);
            portlistPtr++;
            if (portlist.Count == portlistPtr)
            {
                ahalfsecTimer.Enabled = false;
                this.decoderBox.Invoke(new MethodInvoker(() => this.decoderBox.SelectedIndex = 0));
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
                processBox.AppendText(port + " gefunden" + Environment.NewLine);
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
                Seta1halfsecTimer();
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

        private void transmitBtn_Click(object sender, EventArgs e)
        {
            // L!index!ontime!offTime!brightness
            // L!3!27!39!32
            String v0 = decoderBox.SelectedIndex.ToString();
            String v1 = onTime.Value.ToString();
            String v2 = offTime.Value.ToString();
            String v3 = brightness.Value.ToString();
            String v4 = baseTime.Value.ToString();
            String v = "L!" + v0 + '!' + v1 + '!' + v2 + '!' + v3 + '!' + v4;
            bssid = write2Port(v);
        }

        private void decoderBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (bports)
            {
                string command = "DEC" + decoderBox.GetItemText(decoderBox.SelectedItem);
                bssid = write2Port(command);
            }
        }

        private void resetBtn_Click(object sender, EventArgs e)
        {
            if (bports)
            {
                makeReceive = false;
                string command = "RES";
                bssid = write2Port(command);
                lightportsBox.SelectedIndex = 0;
            }
        }
    }
}