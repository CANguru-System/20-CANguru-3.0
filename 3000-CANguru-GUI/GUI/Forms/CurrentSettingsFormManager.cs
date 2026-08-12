using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace CANguru.GUI.Forms
{
    public static class CurrentSettingsFormManager
    {
        private static IDecoderSettingsForm _currentForm;

        public static void Register(IDecoderSettingsForm form)
        {
            _currentForm = form;
        }

        public static void Unregister(IDecoderSettingsForm form)
        {
            if (_currentForm == form)
                _currentForm = null;
        }

        public static bool HasActiveForm => _currentForm != null;

        public static void OnResetAck(byte assignedId, byte decoderType)
        {
            _currentForm?.OnResetAck(assignedId, decoderType);
        }
    }
}
