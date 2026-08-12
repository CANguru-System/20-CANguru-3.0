#pragma once
#include <stdint.h>
#include "generated_constants/protocol_constants_core.h"

class DecoderBase;

extern uint8_t DEVTYPE;
extern uint8_t VERS_HIGH;
extern uint8_t VERS_LOW;


extern DecoderBase* gDecoder;
extern uint8_t Decoder_Mac[6];
extern uint8_t Bridge_Mac[6];
extern bool espNowReady;
extern uint8_t nextType; 
extern bool bridge_is_known;
const uint8_t uid_num = 4;
extern uint8_t uid_device[uid_num];
extern uint8_t hasharr[2];

typedef void (*BeforeRestartCallback)();
extern BeforeRestartCallback beforeRestartCallback;

typedef void (*AfterstartCallback)();
extern AfterstartCallback afterstartCallback;

void coreEspNowInit();                                 
uint8_t aliveGetDecoderID();
bool handleMacDirectFrame(uint8_t feature, uint8_t frame, const uint8_t *data, int len);
bool handleGlobalFrame(uint8_t feature, uint8_t frame, const uint8_t *data, int len);
bool handleTypedFrame(uint8_t feature, uint8_t frame, const uint8_t *data, int len);
bool handleIdBoundFrame(uint8_t feature, uint8_t frame, const uint8_t *data, int len);

void sendRejoin();
void startHelloSequence();
void logMac(const char *prefix, const uint8_t *mac);
void coreEspNowSendCAN2Bridge(const uint8_t *data);

typedef struct {
    const char* (*getLine0)();   // liefert z. B. "CANguru-"
    const char* (*getLine1)();   // liefert z. B. "Booster"
} ConfigProvider_t;
void registerConfigProvider(const ConfigProvider_t* provider);
