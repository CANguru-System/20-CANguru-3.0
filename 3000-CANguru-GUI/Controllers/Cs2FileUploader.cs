using CANguru.Networking;
using CANguru.Views;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace CANguru
{
    public class Cs2FileUploader
    {
        private byte[] fileData = Array.Empty<byte>();
        private int blockSize = 512;
        private int offset = 0;
        private int bytesRead = 0;
        private bool waitingForAck = false;

        private TaskCompletionSource<bool>? uploadTcs;

        public event Action<int, int>? ProgressChanged;   // (offset, total)
        public event Action? UploadCompleted;
        public event Action<string>? UploadError;

        private void StartUpload(string filePath)
        {
            if (!File.Exists(filePath))
            {
                var msg = "Datei nicht gefunden: " + filePath;
                uploadTcs?.TrySetResult(false);
                UploadError?.Invoke(msg);
                return;
            }

            uploadTcs?.TrySetResult(true);

            fileData = File.ReadAllBytes(filePath);
            offset = 0;
            waitingForAck = false;

            SendNextBlock();
        }

        public Task<bool> StartUploadAsync(string filePath)
        {
            if (uploadTcs != null && !uploadTcs.Task.IsCompleted)
                throw new InvalidOperationException("Es läuft bereits ein Upload.");

            uploadTcs = new TaskCompletionSource<bool>();

            // hier startest du den Upload
            StartUpload(filePath);

            return uploadTcs.Task;
        }

        private void SendNextBlock()
        {
            if (waitingForAck)
                return;

            if (offset >= fileData.Length)
            {
                uploadTcs?.TrySetResult(true);
                UploadCompleted?.Invoke();
                return;
            }

            int len = Math.Min(blockSize, fileData.Length - offset);
            byte[] block = new byte[len];
            Array.Copy(fileData, offset, block, 0, len);

            byte[] packet = new byte[5 + len];
            packet[0] = ProtocolConstants.CMD_FILE_BLOCK_CS2;
            packet[1] = (byte)(offset >> 24);
            packet[2] = (byte)(offset >> 16);
            packet[3] = (byte)(offset >> 8);
            packet[4] = (byte)(offset);

            Array.Copy(block, 0, packet, 5, len);

            Form1.Instance.SendToBridge(packet);

            waitingForAck = true;
            bytesRead += len;

            ProgressChanged?.Invoke(bytesRead, fileData.Length);
        }

        public void HandleAck(byte[] data)
        {
            // 1) Packet parsen
            if (!PacketParser.TryParse(data, out var featureId, out var commandId, out var payload))
                return;

            // 2) Nur ACKs für CS2-Dateien akzeptieren
            if (featureId != ProtocolConstants.FEATURE_CS2)
                return;

            if (commandId != ProtocolConstants.CMD_FILE_ACK_CS2)
                return;

            // 3) Payload prüfen
            if (payload.Length < 5)
                return;

            int ackOffset =
                (payload[0] << 24) |
                (payload[1] << 16) |
                (payload[2] << 8) |
                (payload[3]);

            byte status = payload[4];

            // 4) Offset prüfen
            if (ackOffset != offset)
                return;

            // 5) Fehler?
            if (status != 0)
            {
                string msg = $"Bridge meldet Fehler bei Offset {ackOffset}.";
                uploadTcs?.TrySetResult(false);   // Upload fehlgeschlagen, aber kein Crash
                UploadError?.Invoke(msg);         // GUI bekommt Meldung
                return;
            }

            // 6) Alles OK → nächsten Block senden
            waitingForAck = false;
            offset += blockSize;

            SendNextBlock();
        }
    }
}
