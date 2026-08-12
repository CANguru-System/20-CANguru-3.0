public static class PacketBuilder
{
    // Feature | Frame | Payload

    public static byte[] Build(byte featureId, byte commandId, byte[] payload)
    {
        int len = payload?.Length ?? 0;
        var data = new byte[2 + len];

        data[0] = featureId;
        data[1] = commandId;

        if (len > 0)
            Array.Copy(payload, 0, data, 2, len);

        return data;
    }
}
