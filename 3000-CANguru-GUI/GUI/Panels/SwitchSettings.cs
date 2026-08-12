using System;
using System.Collections.Generic;
using System.Text;

namespace CANguru.GUI.Panels
{
    public class SwitchSettings
    {
        public byte AssignedId { get; set; }
        public byte Address { get; set; }
        public byte Delay { get; set; }
        public ushort StepsToEnd { get; set; }
        public byte RightOrLeft { get; set; }
        public byte StepDirection { get; set; }

    }
}
