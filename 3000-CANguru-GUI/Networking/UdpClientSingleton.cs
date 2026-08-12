using CANguru.Models;
using System.ComponentModel;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;

public sealed class UdpClientSingleton
{
    private static bool IsDesigner =>
        LicenseManager.UsageMode == LicenseUsageMode.Designtime ||
        Process.GetCurrentProcess().ProcessName == "devenv";

    private static readonly UdpClientSingleton _inst = new();
    public static UdpClientSingleton Instance => _inst;

    private readonly UdpClient _client;
    private readonly IPEndPoint _remote;

    private volatile DecoderStatus _DecoderStatus = DecoderStatus.Healthy;

    private UdpClientSingleton()
    {
        // Designer darf KEIN Netzwerk initialisieren
        if (IsDesigner)
            return;

        _client = new UdpClient();
        _client.Client.ReceiveTimeout = 500;

        _remote = new IPEndPoint(
            ProtocolConfig.BridgeIP,
            ProtocolConstants.BRIDGE_PORT
        );
    }

    public byte[] SendAndReceive(byte[] packet)
    {
        if (IsDesigner)
            return Array.Empty<byte>();

        _client.Send(packet, packet.Length, _remote);
        var ep = new IPEndPoint(IPAddress.Any, 0);
        return _client.Receive(ref ep);
    }

    public void SendBroadcast(byte[] packet)
    {
        if (IsDesigner)
            return;

        _client.Send(packet, packet.Length,
            new IPEndPoint(IPAddress.Broadcast, ProtocolConfig.BridgePort));
    }

    public void SetDecoderStatus(DecoderStatus state)
    {
        _DecoderStatus = state;
    }

    public void Send(byte[] packet)
    {
        if (IsDesigner)
            return;

        if (_DecoderStatus == DecoderStatus.Lost)
            return;

        _client.Send(packet, packet.Length, _remote);
    }
}
