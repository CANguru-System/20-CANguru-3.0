using System.Net;

public static class ProtocolConfig
{
    public static IPAddress BridgeIP { get; set; } = IPAddress.Broadcast; // IPAddress.Parse("192.168.4.1");
    public const int BridgePort = ProtocolConstants.BRIDGE_PORT;
    public static string ipString = string.Empty;

}
