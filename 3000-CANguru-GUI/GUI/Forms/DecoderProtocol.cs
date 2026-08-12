namespace CANguru.GUI.Forms
{
    using System;
    using System.Collections.Concurrent;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Threading;

    public static class DecoderProtocol
    {
        // Designer-Erkennung
        private static bool IsDesigner =>
            LicenseManager.UsageMode == LicenseUsageMode.Designtime ||
            Process.GetCurrentProcess().ProcessName == "devenv";

        // Antwortspeicher pro (Feature, Command)
        private static readonly ConcurrentDictionary<(byte feature, byte cmd), ResponseWaiter> waiters
            = new ConcurrentDictionary<(byte, byte), ResponseWaiter>();

        // Wird von ProcessIncomingPacket aufgerufen
        public static void RaiseResponse(byte feature, byte cmd, byte[] payload)
        {
            if (IsDesigner)
                return;

            var key = (feature, cmd);

            if (waiters.TryGetValue(key, out var waiter))
            {
                waiter.SetPayload(payload);
            }
        }

        // Generische Payload-Lesefunktion
        public static byte[] ReadPayload(byte feature, byte cmdGet, int expectedLength, byte[] requestPayload)
        {
            if (IsDesigner)
            {
                // Dummy-Daten für Designer, damit Panels laden können
                return new byte[expectedLength];
            }

            var key = (feature, cmdGet);
            var waiter = new ResponseWaiter();

            waiters[key] = waiter;

            try
            {
                var request = PacketBuilder.Build(feature, cmdGet, requestPayload);
                UdpClientSingleton.Instance.Send(request);

                if (!waiter.Wait(500))   // Timeout
                    return null;

                if (waiter.Payload == null || waiter.Payload.Length < expectedLength)
                    return null;

                return waiter.Payload;
            }
            finally
            {
                waiters.TryRemove(key, out _);
            }
        }

        public static ushort ReadUInt16(byte feature, byte cmdGet, ushort defaultValue, byte[] requestPayload)
        {
            if (IsDesigner)
                return defaultValue;

            var payload = ReadPayload(feature, cmdGet, expectedLength: 2, requestPayload: requestPayload);
            if (payload == null || payload.Length < 2)
                return defaultValue;

            return (ushort)((payload[0] << 8) | payload[1]);
        }

        public static ushort ReadUInt16(byte feature, byte cmdGet, ushort defaultValue)
        {
            return ReadUInt16(feature, cmdGet, defaultValue, Array.Empty<byte>());
        }

        // Hilfsklasse für Antwort-Synchronisation
        private class ResponseWaiter
        {
            private readonly AutoResetEvent evt = new AutoResetEvent(false);

            public byte[] Payload { get; private set; }

            public void SetPayload(byte[] payload)
            {
                Payload = payload;
                evt.Set();
            }

            public bool Wait(int timeoutMs)
            {
                if (IsDesigner)
                    return false;

                return evt.WaitOne(timeoutMs);
            }
        }

        public static byte[] RequestAndReloadSettings(byte assignedId, byte decoderType)
        {
            byte feature;
            byte command;
            int expectedLength;

            switch (decoderType)
            {
                case ProtocolConstants.DEVTYPE_SWITCH:
                    feature = ProtocolConstants.FEATURE_SWITCH;
                    command = ProtocolConstants.SWITCH_CMD_GET_SETTINGS;
                    expectedLength = ProtocolConstants.EXP_LNG_SWITCH_SETTINGS;
                    break;

                case ProtocolConstants.DEVTYPE_SIGNAL:
                    feature = ProtocolConstants.FEATURE_SIGNAL;
                    command = ProtocolConstants.SIGNAL_CMD_GET_SETTINGS;
                    expectedLength = ProtocolConstants.EXP_LNG_SIGNAL_SETTINGS;
                    break;

                case ProtocolConstants.DEVTYPE_POWER:
                    feature = ProtocolConstants.FEATURE_POWER;
                    command = ProtocolConstants.POWER_CMD_GET_THRESHOLD;
                    expectedLength = ProtocolConstants.EXP_LNG_POWER_SETTINGS;
                    break;

                case ProtocolConstants.DEVTYPE_RM:
                    feature = ProtocolConstants.FEATURE_FEEDBACK;
                    command = ProtocolConstants.FEEDBACK_CMD_GET_SETTINGS;
                    expectedLength = ProtocolConstants.EXP_LNG_FEEDBACK_SETTINGS;
                    break;

                default:
                    return null;
            }

            // Retry-Logik nach Reset
            for (int i = 0; i < 5; i++)
            {
                var payload = DecoderProtocol.ReadPayload(
                    feature,
                    command,
                    expectedLength,
                    new byte[] { assignedId }
                );

                if (payload != null)
                    return payload;

                Thread.Sleep(100); // Decoder braucht Zeit nach Reset
            }

            return null;
        }
    }
}
