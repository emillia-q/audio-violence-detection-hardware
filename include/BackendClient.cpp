#include "BackendClient.h"

const char* BackendClient::BASE_URL = BACKEND_BASE_URL;
const char* BackendClient::ACTIVATE_URL = BACKEND_ACTIVATE_DEVICE_URL;

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
    return success;
}