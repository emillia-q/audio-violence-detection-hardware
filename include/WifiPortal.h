#pragma once

#include<Arduino.h>
#include<WebServer.h>
#include<DNSServer.h>
#include"secret.h"
#include"HtmlPages.h"

class WifiPortal {
    // HTTP server & DNS server on domain ports
    static WebServer server;
    static DNSServer dnsServer;

    // Received from the html form
    static const char* PARAM_SSID;
    static const char* PARAM_PASS;
    static const char* PARAM_EMAIL;

    static void handleRoot();
    static void handleSave();
    static void handleNotFound();
public:
    static void startConfigurationMode();
};