#pragma once

#include <Arduino.h>
#include<Preferences.h>

class NvsManager {
    static Preferences prefs;
    
    static const char* NAMESPACE;
    static const char* KEY_SSID;
    static const char* KEY_PASS;
    static const char* KEY_EMAIL;
    static const char* KEY_SECRET;
    static const char* KEY_ACTIVATED;
public:
    static bool begin();

    // Device Secret
    static void saveDeviceSecret(const String& secret);
    static bool hasDeviceSecret();
    static String getDeviceSecret();

    // WiFi
    static void saveWiFiCredentials(const String& ssid, const String& pass);
    static bool hasWiFiCredentials();
    static String getWiFiSsid();
    static String getWiFiPass();
    static void clearWiFiConfig();

    // User email
    static void saveUserEmail(const String& email);
    static bool hasUserEmail();
    static String getUserEmail();
    static void clearUserEmail();

    // Activation
    static void setActivated(bool status);
    static bool isActivated();
};