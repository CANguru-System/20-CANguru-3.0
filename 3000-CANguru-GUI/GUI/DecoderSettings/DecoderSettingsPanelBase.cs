using System.Drawing;
using System.Windows.Forms;

namespace CANguru.GUI.DecoderSettings
{
    public class DecoderSettingsPanelBase : UserControl
    {
        public virtual void LoadFromDecoder() { }
        public virtual void SaveToDecoder() { }
    }
}
