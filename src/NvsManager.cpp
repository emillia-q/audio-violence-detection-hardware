#include "NvsManager.h"

Preferences NvsManager::prefs;
const char* NvsManager::NAMESPACE = "device_cfg";
const char* NvsManager::KEY_SSID = "wifi_ssid";
const char* NvsManager::KEY_PASS = "wifi_pass";
const char* NvsManager::KEY_SECRET = "dev_secret";
const char* NvsManager::KEY_ACTIVATED = "activated";

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

bool NvsManager::hasDeviceSecret()
{
    prefs.begin(NAMESPACE, true);
    bool exists = prefs.isKey(KEY_SECRET);
    prefs.end();
    return exists;
}

String NvsManager::getDeviceSecret()
{
    prefs.begin(NAMESPACE, true);
    String keySecret = prefs.getString(KEY_SECRET, "");
    prefs.end();
    return keySecret;
}

void NvsManager::saveWiFiCredentials(const String &ssid, const String &pass)
{
    prefs.begin(NAMESPACE, false);
    prefs.putString(KEY_SSID, ssid);
    prefs.putString(KEY_PASS, pass);
    prefs.end();
}

bool NvsManager::hasWiFiCredentials()
{
    prefs.begin(NAMESPACE, true);
    bool hasSsid = prefs.isKey(KEY_SSID);
    bool hasPass = prefs.isKey(KEY_PASS);
    prefs.end();
    return (hasSsid && hasPass) ? true : false;
}

String NvsManager::getWiFiSsid()
{
    prefs.begin(NAMESPACE, true);
    String ssid = prefs.getString(KEY_SSID, "");
    return ssid;
}

String NvsManager::getWiFiPass()
{
    prefs.begin(NAMESPACE, true);
    String ssid = prefs.getString(KEY_PASS, "");
    return ssid;
}

void NvsManager::clearWiFiConfig()
{
    prefs.begin(NAMESPACE, false);
    prefs.remove(KEY_SSID);
    prefs.remove(KEY_PASS);
    prefs.end();
}

void NvsManager::setActivated(bool status)
{
    prefs.begin(NAMESPACE, false);
    prefs.putBool(KEY_ACTIVATED, status);
    prefs.end();
}

bool NvsManager::isActivated()
{
    prefs.begin(NAMESPACE, true);
    bool activated = prefs.getBool(KEY_ACTIVATED);
    prefs.end();
    return activated;
}
