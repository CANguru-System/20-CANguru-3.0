using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using CANguru.Networking;
using CANguru.Protocol;

namespace CANguru.OTA
{
    public class OtaUploader
    {
        private struct AckStruct
        {
            public ushort Seq;
            public byte Status;
        }

        private readonly ConcurrentQueue<AckStruct> _ackQueue = new();

        private readonly Action<string> _log;
        private readonly Action<int> _setPercent;   // 0..100
        private readonly int _blockSize;
        private readonly int _maxRetries;
        private readonly int _ackTimeoutMs;

        private uint _seq;

        public OtaUploader(
            Action<string> log,
            Action<int> setPercent,
            int blockSize,
            int maxRetries,
            int ackTimeoutMs)
        {
            _log = log;
            _setPercent = setPercent;
            _blockSize = blockSize;
            _maxRetries = maxRetries;
            _ackTimeoutMs = ackTimeoutMs;
        }

        // Wird von außen aus ProcessIncomingPacket() aufgerufen
        public void HandleAckPayload(byte[] payload)
        {
            if (payload == null || payload.Length < 3)
                return;

            var ack = new AckStruct
            {
                Seq = (ushort)(payload[0] | (payload[1] << 8)),
                Status = payload[2]
            };
            _ackQueue.Enqueue(ack);
        }

        public async Task<bool> UploadFirmwareAsync(byte[] firmware, CancellationToken ct)
        {
            int totalSize = firmware.Length;
            int offset = 0;
            _seq = 0;

            // Startblock (seq=0, payload = totalSize)
            byte[] startPayload = BitConverter.GetBytes(totalSize);
            byte[] startFrame = BuildOtaBlock(_seq, startPayload);

            _log("Sende Startblock...");
            if (!await SendBlockWithAckAsync(startFrame, totalSize, offset, startPayload.Length, ct))
            {
                _log("Startblock NICHT bestätigt. Upload abgebrochen.");
                return false;
            }

            _seq++;

            while (offset < totalSize)
            {
                ct.ThrowIfCancellationRequested();

                int len = Math.Min(_blockSize, totalSize - offset);
                byte[] payload = new byte[len];
                Buffer.BlockCopy(firmware, offset, payload, 0, len);

                byte[] frame = BuildOtaBlock(_seq, payload);

                if (!await SendBlockWithAckAsync(frame, totalSize, offset, len, ct))
                {
                    _log("Upload abgebrochen: keine gültigen ACKs mehr.");
                    return false;
                }

                offset += len;
                _seq++;
            }

            _log("Firmware-Upload zur Bridge abgeschlossen.");
            return true;
        }

        private async Task<bool> SendBlockWithAckAsync(
            byte[] frame,
            int totalSize,
            int offset,
            int len,
            CancellationToken ct)
        {
            for (int attempt = 0; attempt < _maxRetries; attempt++)
            {
                ct.ThrowIfCancellationRequested();

                UdpClientSingleton.Instance.Send(frame);

                var ack = await WaitForAckAsync((ushort)(_seq & 0xFFFF), _ackTimeoutMs, ct);
                if (ack == null)
                {
                    _log($"Block {_seq} NICHT bestätigt, Versuch {attempt + 1}/{_maxRetries}");
                    continue;
                }

                if (ack.Value.Status == 0)
                {
                    int sent = offset + len;
                    int percent = (int)(sent * 100.0 / totalSize);
                    _setPercent(Math.Min(100, percent));
                    return true;
                }

                if (ack.Value.Status == 1)
                {
                    _log($"Block {_seq}: RETRY angefordert.");
                    continue;
                }

                if (ack.Value.Status == 2)
                {
                    _log($"Block {_seq}: FATAL ERROR.");
                    return false;
                }
            }

            return false;
        }

        private async Task<AckStruct?> WaitForAckAsync(ushort expectedSeq, int timeoutMs, CancellationToken ct)
        {
            var sw = Stopwatch.StartNew();

            while (sw.ElapsedMilliseconds < timeoutMs)
            {
                ct.ThrowIfCancellationRequested();

                if (_ackQueue.TryDequeue(out var ack))
                {
                    if (ack.Seq == expectedSeq)
                        return ack;
                }

                await Task.Delay(1, ct);
            }

            return null;
        }

        private static byte[] BuildOtaBlock(uint seq, byte[] payload)
        {
            ushort len = (ushort)payload.Length;
            ushort crc = Crc16(payload);

            byte[] frame = new byte[1 + 4 + 2 + 2 + len];

            frame[0] = ProtocolConstants.OTA_UPLOAD_BLOCK;

            Buffer.BlockCopy(BitConverter.GetBytes(seq), 0, frame, 1, 4);
            Buffer.BlockCopy(BitConverter.GetBytes(len), 0, frame, 5, 2);
            Buffer.BlockCopy(BitConverter.GetBytes(crc), 0, frame, 7, 2);
            Buffer.BlockCopy(payload, 0, frame, 9, len);

            return frame;
        }

        private static ushort Crc16(byte[] data)
        {
            ushort crc = 0xFFFF;

            for (int i = 0; i < data.Length; i++)
            {
                crc ^= (ushort)(data[i] << 8);

                for (int b = 0; b < 8; b++)
                {
                    if ((crc & 0x8000) != 0)
                        crc = (ushort)((crc << 1) ^ 0x1021);
                    else
                        crc <<= 1;
                }
            }

            return crc;
        }
    }
}
