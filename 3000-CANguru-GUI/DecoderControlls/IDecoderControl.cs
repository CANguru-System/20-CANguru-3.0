using System.Windows.Forms;
using CANguru.DecoderModels;

namespace CANguru.DecoderControls
{
    public interface IDecoderControl
    {
        void SetModel(IDecoderModel model);
        UserControl AsUserControl();
    }
}
