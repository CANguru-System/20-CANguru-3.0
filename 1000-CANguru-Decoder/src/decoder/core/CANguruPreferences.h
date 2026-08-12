#pragma once
#include <Preferences.h>

class CANguruPreferences
{
public:
 CANguruPreferences(const char* ns)
    : _ns(ns)
    {
        // NICHT: prefs.begin(ns, false);
    }

    void init()
    {
        prefs.begin(_ns, false);   // jetzt erst NACH setup()
    }
    // Unsigned Integer
    void putUInt(const char *key, uint32_t value)
    {
        prefs.putUShort(key, value);
    }

    uint32_t getUInt(const char *key, uint32_t defaultValue = 0)
    {
        return prefs.getUShort(key, defaultValue);
    }

    // Boolean
    void putBool(const char *key, bool value)
    {
        prefs.putBool(key, value);
    }

    bool getBool(const char *key, bool defaultValue = false)
    {
        return prefs.getBool(key, defaultValue);
    }

    // String
    void putString(const char *key, const String &value)
    {
        prefs.putString(key, value);
    }

    String getString(const char *key, const String &defaultValue = "")
    {
        return prefs.getString(key, defaultValue);
    }

    // Byte
    void putByte(const char *key, uint8_t value)
    {
        prefs.putUChar(key, value);
    }

    uint8_t getByte(const char *key, uint8_t defaultValue = 0)
    {
        return prefs.getUChar(key, defaultValue);
    }

    void putBytes(const char *key, const void *data, size_t len)
    {
        prefs.putBytes(key, data, len);
    }

    size_t getBytes(const char *key, void *data, size_t maxLen)
    {
        return prefs.getBytes(key, data, maxLen);
    }

    bool isKey(const char *key)
    {
        return prefs.isKey(key);
    }

private:
    const char* _ns;
    Preferences prefs;
};
