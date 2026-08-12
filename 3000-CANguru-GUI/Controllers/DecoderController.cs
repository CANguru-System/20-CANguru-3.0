using System;
using System.Windows.Forms;
using CANguru.DecoderModels;
using CANguru.DecoderControls;

namespace CANguru.Controllers
{
    public partial class DecoderController : Form
    {
        public DecoderController()
        {
            InitializeComponent();
        }

        public void LoadDecoder(IDecoderModel model, IDecoderControl control)
        {
            control.SetModel(model);

            panelContent.Controls.Clear();
            panelContent.Controls.Add(control.AsUserControl());
        }

        private Panel panelContent;

        private void InitializeComponent()
        {

        }
    }
}
