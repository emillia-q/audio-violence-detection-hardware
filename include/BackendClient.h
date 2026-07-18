#pragma once

#include<WiFi.h>
#include<HTTPClient.h>
#include"NvsManager.h"
#include"secret.h"

class BackendClient {
    static const char* BASE_URL;
    static const char* ACTIVATE_URL;
    static const char* AUTH_URL;

public:
    static bool activateDevice();
    static bool authenticateDevice();
};