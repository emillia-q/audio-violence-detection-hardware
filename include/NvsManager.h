#pragma once

#include <Arduino.h>
#include<Preferences.h>

class NvsManager {
    static Preferences prefs;
    
    static const char* NAMESPACE;
    static const char* KEY_SSID;
    static const char* KEY_PASS;
    static const char* KEY_SECRET;
    static const char* KEY_ACTIVATED;
public:
    static bool begin();
    static void saveDeviceSecret(const String& secret);
    static bool hasDeviceSecret();
    static String getDeviceSecret();
};