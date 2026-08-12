using System;
using System.Collections.Generic;
using System.Text;

namespace CANguru.Networking
{
    public static class GuiLogger
    {
        public static event Action<string>? MessageLogged;

        public static void Log(string text)
        {
            MessageLogged?.Invoke(text);
        }
    }
}
// Aufruf: GuiLogger.Log("Dies ist eine Log-Nachricht.");