#include "NvsManager.h"

Preferences NvsManager::prefs;
const char* NvsManager::NAMESPACE = "device_cfg";
const char* NvsManager::KEY_SSID = "wifi_ssid";
const char* NvsManager::KEY_PASS = "wifi_pass";
const char* NvsManager::KEY_SECRET = "dev_secret";
const char* NvsManager::KEY_ACTIVATED = "activated";

NvsManager::NvsManager()
{
}

bool NvsManager::begin()
{
    bool success = prefs.begin(NAMESPACE, false);
    prefs.end();
    return success;
}

void NvsManager::saveDeviceSecret(const String &secret)
{
    prefs.begin(NAMESPACE, false);
    prefs.putString(KEY_SECRET, secret);
    prefs.end();
}
