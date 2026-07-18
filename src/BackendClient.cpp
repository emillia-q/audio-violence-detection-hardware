#include "BackendClient.h"

const char* BackendClient::BASE_URL = BACKEND_BASE_URL;
const char* BackendClient::ACTIVATE_URL = BACKEND_ACTIVATE_DEVICE_URL;
const char* BackendClient::AUTH_URL = BACKEND_AUTH_DEVICE_URL;

bool BackendClient::activateDevice()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("No wifi connection. Cannot activate device.");
        return false;
    }

    // Create full url & get data to send
    String activateUrl = String(BASE_URL) + String(ACTIVATE_URL);
    String macAddress = WiFi.macAddress();
    String deviceSecret = NvsManager::getDeviceSecret();
    String email = NvsManager::getUserEmail();

    if (email.length() == 0) {
        Serial.println("No email saved in NVS");
        return false;
    }

    HTTPClient http;
    http.begin(activateUrl);
    http.addHeader("Content-Type", "application/json");

    // Create json & response code
    String jsonPayload = "{\"macAddress\":\"" + macAddress +
                            "\",\"deviceSecret\":\"" + deviceSecret + 
                            "\",\"email\":\"" + email + "\"}";
    int httpResponseCode = http.POST(jsonPayload);
    bool success = false;

    if (httpResponseCode > 0) {
        switch (httpResponseCode) {
            case 204:
                Serial.println("204: Activated and paired an IoT device");
                NvsManager::setActivated(true);
                success = true;
                break;
                
            case 400:
                Serial.println("400: Invalid request payload or validation failed");
                break;
                
            case 401:
                Serial.println("401: Unauthorized, Invalid device secret");
                break;
                
            case 404:
                Serial.println("404: Device or user not found");
                break;
                
            case 409: // Shouldn't happen for now
                Serial.println("409: Conflict, Device is already assigned to a user");
                break;
                
            default:
                Serial.print("Unexpected status code: ");
                Serial.println(httpResponseCode);
                break;
        }
    } else {
        // Physical error
        Serial.print("Connection failed! Error code: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
    return success;
}

bool BackendClient::authenticateDevice()
{
    // Create full url & get data to send
    String activateUrl = String(BASE_URL) + String(AUTH_URL);
    String macAddress = WiFi.macAddress();
    String deviceSecret = NvsManager::getDeviceSecret();

    HTTPClient http;
    http.begin(activateUrl);
    http.addHeader("Content-Type", "application/json");

    // Create json & response code
    String jsonPayload = "{\"macAddress\":\"" + macAddress +
                            "\",\"deviceSecret\":\"" + deviceSecret + "\"}";
    int httpResponseCode = http.POST(jsonPayload);
    bool success = false;

    if (httpResponseCode > 0) {
        switch (httpResponseCode) {
            case 200:
                Serial.println("200: Device successfully authenticated");
                
                success = true;
                break;
                
            case 400:
                Serial.println("400: Invalid request payload or validation failed");
                break;
                
            case 401:
                Serial.println("401: Unauthorized, Invalid MAC address or device secret key");
                break;
                
            default:
                Serial.print("Unexpected status code: ");
                Serial.println(httpResponseCode);
                break;
        }
    } else {
        // Physical error
        Serial.print("Connection failed! Error code: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
    return success;
}
