#include "PacketHandler.h"

ParsedPacket parsePacket(const uint8_t* data, size_t len)
{
    // Feature | Frame | Payload
    ParsedPacket p;

    if (len < 2)
        return p;

    p.featureId = data[0];
    p.commandId = data[1];

    if (len > 2)
        p.payload.assign(data + 2, data + len);

    p.valid = true;
    return p;
}

std::vector<uint8_t> buildPacket(uint8_t featureId,
                                 uint8_t commandId,
                                 const uint8_t* payload,
                                 uint8_t payloadLen)
{
    // Feature | Frame | Payload
    std::vector<uint8_t> data;
    data.reserve(2 + payloadLen);

    data.push_back(featureId);
    data.push_back(commandId);

    if (payloadLen > 0)
        data.insert(data.end(), payload, payload + payloadLen);

    return data;
}
