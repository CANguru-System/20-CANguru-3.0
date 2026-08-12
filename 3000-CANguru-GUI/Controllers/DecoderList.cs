using CANguru.Models;
using System;
using System.Collections.Generic;
using System.Linq;

namespace CANguru.Controllers
{
    public class DecoderList
    {
        public List<CANguruDecoder> Decoders { get; } = new();

        public event Action? DecodersChanged;

        public void UpdateDecoder(CANguruDecoder dec)
        {
            var existing = Decoders.FirstOrDefault(d => d.AssignedId == dec.AssignedId);

            if (existing == null)
            {
                Decoders.Add(dec);
            }
            else
            {
                existing.Mac = dec.Mac;
                existing.Type = dec.Type;
                existing.value0 = dec.value0;
                existing.value1 = dec.value1;
                existing.IsReady = dec.IsReady;
                existing.LastSeen = dec.LastSeen;
            }

            DecodersChanged?.Invoke();
        }
    }
}
