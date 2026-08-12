using System;
using System.Collections.Generic;
using System.Text;

namespace CANguru.Protocol
{
    public class AliveManager
    {
        public event Action<int, int, int, int> AliveSummaryReceived;

        private int lastPower = -1;
        private int lastSwitch = -1;
        private int lastSignal = -1;
        private int lastFeedback = -1;

        public void ProcessAliveSummary(int power, int sw, int signal, int feedback)
        {
            AliveSummaryReceived?.Invoke(power, sw, signal, feedback);
        }
    }
}
