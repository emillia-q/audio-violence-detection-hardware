#include "WifiPortal.h"

WebServer WifiPortal::server(80);
DNSServer WifiPortal::dnsServer;
const char* WifiPortal::PARAM_SSID = "ssid";
const char* WifiPortal::PARAM_PASS = "password";
const char* WifiPortal::PARAM_EMAIL = "email";

void WifiPortal::handleRoot()
{
    server.send(200, "text/html", SETUP_HTML);
}

void WifiPortal::handleSave()
{
}

void WifiPortal::handleNotFound()
{
    // Intercepts queries & redirect to home page
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    server.send(302, "text/plain", "");
}

void WifiPortal::startConfigurationMode()
{
    // Configure esp as access point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);

    // Configure captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());

    // Routing
    // Root page
    server.on("/", HTTP_GET, handleRoot); 
    // Saved config page
    server.on("/save", HTTP_POST, handleSave);
    // Any other path
    server.onNotFound(handleNotFound);

    // Start the server
    server.begin();
}