#pragma once

#include<Arduino.h>
#include<WebServer.h>

class WifiPortal {
    // www server, listens on the standard HTTP port
    static WebServer server;

    // Received from the html form
    static const char* PARAM_SSID;
    static const char* PARAM_PASS;
    static const char* PARAM_EMAIL;
};