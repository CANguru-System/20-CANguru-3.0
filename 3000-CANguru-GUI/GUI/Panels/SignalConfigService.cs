using CANguru.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Text;

namespace CANguru.GUI.Panels
{
    public class SignalConfigService
    {
        private readonly byte assignedId;

        public SignalConfigService(byte assignedId)
        {
            this.assignedId = assignedId;
        }
        public void SendSlowMove(bool toLeft)
        {
            var payload = new byte[]
            {
        assignedId,
        (byte)(toLeft ? 0 : 1)   // 0 = links, 1 = rechts
            };

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_MOVE2START,
                payload
            );

            UdpClientSingleton.Instance.Send(packet);
        }

        public void SetZeroPoint()
        {
            var payload = new byte[] { assignedId };

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_SET_ZERO,
                payload
            );

            UdpClientSingleton.Instance.Send(packet);
        }

        public void SetEndPoint()
        {
            var payload = new byte[] { assignedId };

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_SET_END,
                payload
            );

            UdpClientSingleton.Instance.Send(packet);
        }

        public void StartTestRun()
        {
            var payload = new byte[] { assignedId };

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_TEST_RUN,
                payload
            );

            UdpClientSingleton.Instance.Send(packet);
        }

        public SignalSettings GetSettings()
        {
            byte[] payload = DecoderProtocol.ReadPayload(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_GET_SETTINGS,
                expectedLength: 7,
                requestPayload: new byte[] { assignedId }   // <<< WICHTIG!
            );

            if (payload == null || payload.Length < 7)
                return null;

            return new SignalSettings
            {
                AssignedId = payload[0],
                Address = payload[1],
                Delay = payload[2],
                StepsToEnd = (ushort)((payload[3] << 8) | payload[4]),
                RightOrLeft = payload[5],
                StepDirection = payload[6]
            };
        }
        public SignalSettings GetSettingsWithRetry(int retries = 3, int delayMs = 80)
        {
            for (int attempt = 1; attempt <= retries; attempt++)
            {
                var s = GetSettings();

                if (s != null)
                    return s;

                // Optional: Logging
                // Console.WriteLine($"GetSettings retry {attempt}/{retries} failed.");

                Thread.Sleep(delayMs);
            }

            return null;
        }

        public void SetSettings(SignalSettings s)
        {
            var payload = new byte[]
            {
            s.AssignedId,
            s.Address,
            s.Delay,
            (byte)(s.StepsToEnd >> 8),
            (byte)(s.StepsToEnd & 0xFF),
            s.RightOrLeft,
            s.StepDirection
            };

            var packet = PacketBuilder.Build(
                ProtocolConstants.FEATURE_SIGNAL,
                ProtocolConstants.SIGNAL_CMD_SET_SETTINGS,
                payload
            );

            UdpClientSingleton.Instance.Send(packet);
        }
    }
}
