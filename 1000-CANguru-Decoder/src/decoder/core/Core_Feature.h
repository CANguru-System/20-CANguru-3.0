#pragma once

#include <stdint.h>

// Liefert true, wenn es sich um einen Feature-Frame handelt,
// der an den konkreten Decoder (z.B. PowerDecoder) weitergereicht werden soll.
bool isFeatureFrame(uint8_t type);
