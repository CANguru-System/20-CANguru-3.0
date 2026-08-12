#include <Arduino.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include "protocol_constants_core.h"
#include "Cs2FileReceiver.h"
#include "gui_proc.h"

static File cs2File;
static bool receiving = false;
static uint32_t expectedOffset = 0;

void Cs2FileReceiver::handlePacket(const uint8_t *data, int len)
{
    if (len < 1)
        return;

    switch (data[0])
    {
    case CMD_FILE_BLOCK_CS2:
        handleFileBlock(data, len);
        break;

    default:
        break;
    }
}

void Cs2FileReceiver::handleFileBlock(const uint8_t *data, int len)
{
    if (len < 6)
        return;

    uint32_t offset =
        (data[1] << 24) |
        (data[2] << 16) |
        (data[3] << 8) |
        (data[4]);

    const uint8_t *block = &data[5];
    int blockLen = len - 5;
    // Erster Block?
    if (!receiving)
    {
        receiving = true;
        expectedOffset = 0;

        // Datei neu anlegen
        /* The `cs2File` variable is a static File object used to store the file being received during
        the CS2 file transfer process. It is opened for writing with the name "/lokomotive.cs2" in
        the LittleFS file system. The received file blocks are written to this file as they are
        received, and the file is flushed after each block write to ensure the data is written to
        the file system. */
        cs2File = LittleFS.open("/lokomotive.cs2", "w");
        if (!cs2File)
        {
            Serial.println("Fehler: Datei konnte nicht geoeffnet werden!");
            sendAck(offset, 1);
            return;
        }

    }

    // Offset prüfen
    if (offset != expectedOffset)
    {
        Serial.printf("Fehler: Unerwarteter Offset! Erwartet %u, erhalten %u\r\n",
                      expectedOffset, offset);
        sendAck(offset, 1);
        return;
    }

    // Block schreiben
    int originalBlockLen = blockLen;

    // BOM entfernen
    if (blockLen >= 3 &&
        block[0] == 0xEF &&
        block[1] == 0xBB &&
        block[2] == 0xBF)
    {
        block += 3;
        blockLen -= 3;
    }

    // Schreiben
    cs2File.write(block, blockLen);
    cs2File.flush();

    // Offset so erhöhen, wie der Sender es erwartet
    expectedOffset += originalBlockLen;

    sendAck(offset, 0);
}

void Cs2FileReceiver::sendAck(uint32_t offset, uint8_t status)
{
    uint8_t payload[5];
    payload[0] = (offset >> 24) & 0xFF;
    payload[1] = (offset >> 16) & 0xFF;
    payload[2] = (offset >> 8) & 0xFF;
    payload[3] = offset & 0xFF;
    payload[4] = status;

    auto packet = buildPacket(
        FEATURE_CS2,
        CMD_FILE_ACK_CS2,
        payload,
        sizeof(payload)
    );

    sendPacketToGUI(packet);
}
