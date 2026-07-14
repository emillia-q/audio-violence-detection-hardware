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
    // Check if form fields came in request
    if (server.hasArg(PARAM_SSID) && server.hasArg(PARAM_PASS) && server.hasArg(PARAM_EMAIL)) {
        // Extract values
        String receivedSsid = server.arg(PARAM_SSID);
        String receivedPass = server.arg(PARAM_PASS);
        String receivedEmail = server.arg(PARAM_EMAIL);

        // Send success page
        server.send(200, "text/html", SUCCESS_HTML);

        // Save wifi config to NVS
        NvsManager::saveWiFiCredentials(receivedSsid, receivedPass);
        NvsManager::saveUserEmail(receivedEmail);
        // Credentials updated- device needs to re-register with the backend on next boot
        NvsManager::setActivated(false);

        // Wait & restart esp
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Bad Request: Missing form fields");
    }
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

void WifiPortal::handleClient()
{
    dnsServer.processNextRequest();
    server.handleClient();
}

bool WifiPortal::connectToSavedWifi()
{
    String ssid = NvsManager::getWiFiSsid();
    String pass = NvsManager::getWiFiPass();

    if (ssid.length() == 0) {
        Serial.println("No SSID saved in NVS");
        return false;
    }

    // Turn off AP mode & connect as client
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.print("Connecting to: ");
    Serial.println(ssid);
    // Wait for connection (max 15s)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected successfully");
        return true;
    } else {
        Serial.println("\nConnection timeout");
        return false;
    }
}
