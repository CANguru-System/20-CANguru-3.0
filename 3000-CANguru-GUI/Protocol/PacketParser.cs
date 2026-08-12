public static class PacketParser
{
    // Feature | Frame | Payload

    public static bool TryParse(byte[] data, out byte featureId, out byte commandId, out byte[] payload)
    {
        featureId = 0;
        commandId = 0;
        payload = Array.Empty<byte>();

        if (data == null || data.Length < 2)
            return false;

        featureId = data[0];
        commandId = data[1];

        int len = data.Length - 2;

        if (len > 0)
        {
            payload = new byte[len];
            Array.Copy(data, 2, payload, 0, len);
        }

        return true;
    }
}
