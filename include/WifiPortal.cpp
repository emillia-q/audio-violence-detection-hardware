#include "WifiPortal.h"

WebServer WifiPortal::server(80);
const char* WifiPortal::PARAM_SSID = "ssid";
const char* WifiPortal::PARAM_PASS = "password";
const char* WifiPortal::PARAM_EMAIL = "email";

void WifiPortal::handleRoot()
{
}

void WifiPortal::handleSave()
{
}

void WifiPortal::startConfigurationMode()
{
    // Configure esp as access point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);

    // Routing
    // Root page
    server.on("/", HTTP_GET, handleRoot); 
    // Saved config page
    server.on("/save", HTTP_POST, handleSave);

    // Start the server
    server.begin();
}