using CANguru.Controllers;
using CANguru.Models;
using CANguru.Communication;
using CANguru.Controllers;
using CANguru.Models;
using System;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Threading;

namespace CANguru.Communication
{
    public class UdpBridgeClient : IDisposable
    {
        private readonly UdpClient _client;
        private readonly IPEndPoint _bridgeEndPoint;
        private readonly DecoderList _decoderController;
        private readonly Thread _recvThread;
        private bool _running;

        public UdpBridgeClient(DecoderList decoderController, string bridgeIp, int bridgePort)
        {
            _decoderController = decoderController;
            _bridgeEndPoint = new IPEndPoint(IPAddress.Parse(bridgeIp), bridgePort);
            _client = new UdpClient(0); // beliebiger lokaler Port
            _running = true;
            _recvThread = new Thread(ReceiveLoop) { IsBackground = true };
            _recvThread.Start();
        }

        private void ReceiveLoop()
        {
            var any = new IPEndPoint(IPAddress.Any, 0);

            while (_running)
            {
                try
                {
                    var data = _client.Receive(ref any);
                }
                catch
                {
                    if (!_running) break;
                }
            }
        }

        public void Dispose()
        {
            _running = false;
            try { _client.Close(); } catch { }
        }
    }
}
