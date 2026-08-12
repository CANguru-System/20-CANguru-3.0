using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using CANguru.Views;

namespace CANguru
{
    internal static class Program
    {
        private static Mutex mutex = null;
        private const string AppMutexName = "MeineWunderbareCANguruGUI";

        [DllImport("user32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll")]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        private const int SW_RESTORE = 9;

        [STAThread]
        static void Main()
        {
            bool createdNew;
            mutex = new Mutex(true, AppMutexName, out createdNew);

            if (!createdNew)
            {
                BringExistingInstanceTonullpkt();
                return;
            }

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            // HIER deinen Form-Namen eintragen:
            Application.Run(new Form1());
        }

        private static void BringExistingInstanceTonullpkt()
        {
            var current = System.Diagnostics.Process.GetCurrentProcess();
            var processes = System.Diagnostics.Process.GetProcessesByName(current.ProcessName);

            foreach (var p in processes)
            {
                if (p.Id != current.Id)
                {
                    IntPtr handle = p.MainWindowHandle;

                    if (handle != IntPtr.Zero)
                    {
                        ShowWindow(handle, SW_RESTORE);
                        SetForegroundWindow(handle);
                    }
                    break;
                }
            }
        }
    }
}