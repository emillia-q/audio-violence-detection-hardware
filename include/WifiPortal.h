#pragma once

#include<Arduino.h>
#include<WebServer.h>
#include<DNSServer.h>
#include"secret.h"
#include"HtmlPages.h"
#include"NvsManager.h"

class WifiPortal {
    // HTTP server & DNS server on domain ports
    static WebServer server;
    static DNSServer dnsServer;

    // Received from the html form
    static const char* PARAM_SSID;
    static const char* PARAM_PASS;

    static void handleRoot();
    static void handleSave();
    static void handleNotFound();
public:
    // Hotspot
    static void startConfigurationMode();
    static void handleClient();

    // WiFi
    static bool connectToSavedWifi();
};