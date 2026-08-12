using System;
using System.Collections.Generic;
using System.Text;

namespace CANguru.GUI.Forms
{
    public interface IDecoderSettingsForm
    {
        void OnResetAck(byte assignedId, byte decoderType);
    }
    public interface IDecoderListViewItem
    {
        (string Type, string Mac, string Id, string uid, string Details) ToListViewColumns();
    }
}
