#include "protocol_constants_core.h"
#include "DecoderBase.h"
#include "DecoderFactory.h"

#if DECODER_TYPE == DEVTYPE_SIGNAL
#include "../features/signal/SignalDecoder.h"
#elif DECODER_TYPE == DEVTYPE_SWITCH
#include "../features/switch/SwitchDecoder.h"
#elif DECODER_TYPE == DEVTYPE_POWER
#include "../features/power/PowerDecoder.h"
#elif DECODER_TYPE == DEVTYPE_RM
#include "../features/feedback/FeedbackDecoder.h"
#endif

DecoderBase* createDecoder() {
#if DECODER_TYPE == DEVTYPE_SIGNAL
    return new SignalDecoder();
#elif DECODER_TYPE == DEVTYPE_SWITCH
    return new SwitchDecoder();
#elif DECODER_TYPE == DEVTYPE_POWER
    return new PowerDecoder();
#elif DECODER_TYPE == DEVTYPE_RM
    return new FeedbackDecoder();
#else
    return nullptr;
#endif
}
