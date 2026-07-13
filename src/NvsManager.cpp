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

void NvsManager::init()
{
    prefs.begin(NAMESPACE, false);
    prefs.end();
}
