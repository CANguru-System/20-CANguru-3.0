#pragma once
#include <stdint.h>

extern const char* key_id;
extern const char* key_otaPending;
extern const char* key_bridgeMac;

void coreConfigInit();
bool coreConfigIsOtaPending();
void coreConfigClearOtaPending();
void coreConfigSetOtaPending(bool v);
