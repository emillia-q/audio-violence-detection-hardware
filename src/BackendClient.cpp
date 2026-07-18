#include "BackendClient.h"

const char* BackendClient::BASE_URL = BACKEND_BASE_URL;
const char* BackendClient::ACTIVATE_URL = BACKEND_ACTIVATE_DEVICE_URL;
const char* BackendClient::AUTH_URL = BACKEND_AUTH_DEVICE_URL;
const char* BackendClient::SEND_ALERT_URL = BACKEND_SEND_ALERT_URL;

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
            case 200: {
                Serial.println("200: Device successfully authenticated");
                String responseBody = http.getString();
                String jwtToken = "";

                // Trim token
                int tokenKeyIdx = responseBody.indexOf("\"token\":\"");
                if (tokenKeyIdx != -1) {
                    int startPos = tokenKeyIdx + 9;
                    int endPos = responseBody.indexOf("\"", startPos); // Closing quotation

                    if (endPos != -1) {
                        jwtToken = responseBody.substring(startPos, endPos);
                        jwtToken.trim();
                    }
                }
                NvsManager::saveToken(jwtToken);
                success = true;
            } break;
                
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

bool BackendClient::sendAlert(int& statusCode)
{
    // Create full url & auth header string
    String activateUrl = String(BASE_URL) + String(SEND_ALERT_URL);
    String jwtToken = NvsManager::getToken();
    String authHeader = "Bearer " + jwtToken;

    HTTPClient http;
    http.begin(activateUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", authHeader.c_str());

    int httpResponseCode = http.POST(""); // Empty request
    statusCode = httpResponseCode;
    bool success = false;

    if (httpResponseCode > 0) {
        switch (httpResponseCode) {
            case 204: {
                Serial.println("204: Alert sent successfully");
                success = true;
            } break;
                
            case 401:
                Serial.println("401: Unauthorized: token possibly expired");
                break;
                
            case 403:
                Serial.println("403: Forbidden: Required role 'DEVICE' is missing");
                break;

            case 404:
                Serial.println("404: Device not found");
                break;
            
            case 422:
                Serial.println("422: User not assigned to the device");
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
