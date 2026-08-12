using CANguru.Views;
using System;
using System.Collections.Generic;
using System.Text;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace CANguru.Controllers
{
    public class PowerDecoderController
    {
        public int Current0_mA { get; private set; }
        public int Current1_mA { get; private set; }
        public int Threshold_mA { get; set; } = 1000;

        public event Action ValuesUpdated;
        public event Action<int> ShutdownTriggered;

        public void UpdateCurrent(int channel, int mA)
        {
            if (channel == 0)
                Current0_mA = mA;
            else
                Current1_mA = mA;

            ValuesUpdated?.Invoke();
        }

        public void TriggerShutdown(int channel)
        {
            ShutdownTriggered?.Invoke(channel);
        }

        public void SendThreshold()
        {
            byte[] frame = new byte[4];
            frame[0] = ProtocolConstants.POWER_CMD_SET_THRESHOLD;
            frame[1] = (byte)SelectedDecoderId;
            frame[2] = (byte)(Threshold_mA >> 8);
            frame[3] = (byte)(Threshold_mA & 0xFF);
            Form1.Instance.SendToBridge(frame);
        }

        public int SelectedDecoderId { get; set; }
    }
}
