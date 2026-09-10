#include <Arduino.h>
#include <esp_heap_caps.h>
#include "Inmp441.h"
#include "AudioBuffer.h"
#include"secret.h"
#include"NvsManager.h"
#include"WifiPortal.h"
#include"BackendClient.h"
#include "Led.h"
#include "Button.h"
#include <audio-violence-detection_inferencing.h>

// Pin configuration

// RED LED
#define RED_LED 10

// Button
#define BUTTON_PIN 8

// INMP441
#define MIC_WS   15         // Word Select
#define MIC_SD   13         // Serial Data
#define MIC_SCK  4          // Serial Clock
#define I2S_PORT I2S_NUM_0  // Use first available I2S port
#define BUFFER_LEN 256

// Object instances
Inmp441 mic(MIC_WS, MIC_SD, MIC_SCK, I2S_PORT);
AudioBuffer audioBuffer;
ErrorCode currentError = ErrorCode::NONE;
Led led(RED_LED);
ButtonEvent buttonEvent = ButtonEvent::NONE;
Button button(BUTTON_PIN);

// Global buffers allocated from PSRAM at runtime.
float* modelInputBuffer = nullptr;

// Program variables
int statusCode = 0;
unsigned long lastWifiRetryTime = 0; 
unsigned long lastActivationRetryTime = 0; 
unsigned long lastAlertRetryTime = 0; 
unsigned long alertTimestamp = 0;
bool isConfigMode = false;
bool alertSent = true;
bool deviceAuthenticated = true;
bool hardwareFailed = false;

// Constants
const unsigned long RETRY_INTERVAL = 10000;
const unsigned long ALERT_TIMEOUT = 120000;

void setup() {
  Serial.begin(115200);

  modelInputBuffer = (float*)heap_caps_malloc(32000 * sizeof(float), MALLOC_CAP_SPIRAM);
  if (modelInputBuffer == nullptr) {
    Serial.println("Failed to allocate model input buffer from PSRAM");
    currentError = ErrorCode::HARDWARE_ERROR;
    hardwareFailed = true;
  }

  // NVS configuration
  if (!NvsManager::begin()) {
    Serial.println("NVS Error");
    currentError = ErrorCode::HARDWARE_ERROR;
  }
  
  // First check the device secret key
  if (!NvsManager::hasDeviceSecret())
    NvsManager::saveDeviceSecret(DEVICE_SECRET);

  // Then run the hotspot mode
  if (!NvsManager::hasWiFiCredentials()) {
    isConfigMode = true;
    WifiPortal::startConfigurationMode();
  } else {
    // Hardware initialization
    // Mic init
    if(mic.begin())
      Serial.println("INMP441 initialized successfully");
    else {
      Serial.println("Failed to configure INMP441");
      currentError = ErrorCode::HARDWARE_ERROR;
    }

    // Connect to WiFi
    WifiPortal::connectToSavedWifi();
  }

  hardwareFailed = currentError == ErrorCode::HARDWARE_ERROR;
}

void loop() {

  // Update led message
  led.update();

  // Handle button
  buttonEvent = button.getEvent();

  // Display error
  if (buttonEvent == ButtonEvent::SHORT_CLICK) {
    led.errorMessage(currentError);
  }

  // Clear WiFi config & restart the device that will enter hotspot mode
  if (buttonEvent == ButtonEvent::LONG_HOLD) {
    NvsManager::clearWiFiConfig();
    delay(2000);
    ESP.restart();
  }

  // Checks if user has connected to hotspot
  if (isConfigMode) {
    WifiPortal::handleClient();
    delay(1);
    return;
  }

  // Early return when hardware failed
  if (hardwareFailed)
    return;

  // WiFi connection check
  if (WiFi.status() != WL_CONNECTED) {
    currentError = ErrorCode::WIFI_ERROR;

    // Try to connect again after 10s
    if (millis() - lastWifiRetryTime >= RETRY_INTERVAL) {
      lastWifiRetryTime = millis();
      Serial.println("No wifi. Trying to connect again.");
      WifiPortal::connectToSavedWifi();
    }

    return; // Disable CNN processing until there is WiFi connectuon
  } else {
    currentError = ErrorCode::NONE;
  }

  // Activation status check
  if (!NvsManager::isActivated()) {
    // Set http error only when device has WiFi connection
    if (WiFi.status() == WL_CONNECTED) {
      if (millis() - lastActivationRetryTime >= RETRY_INTERVAL && !led.isLedBusy()) {
        lastActivationRetryTime = millis();
        Serial.println("Device not activated. Trying to activate.");
        if (!BackendClient::activateDevice()) {
          currentError = ErrorCode::HTTP_ERROR;
          Serial.println("Failed to activate device.");
        } else {
          Serial.println("Device activated");
        }
      }
    }
    return; // Disable CNN processing until there is WiFi connectuon
  }

  // CNN processing
  int16_t sample_buffer[BUFFER_LEN];
  int samples_read = mic.readSamples(sample_buffer, BUFFER_LEN);
  for (int i = 0; i < samples_read; i++)
    audioBuffer.addSample(sample_buffer[i]);

  if (audioBuffer.isWindowReady() && !led.isLedBusy()) {
    audioBuffer.extractAndNormalizeWindow(modelInputBuffer);

    // MFCC extraction

    // Model prediction

    // Check if violence was detected & send alert
    ///if () {
      alertSent = false;
      alertTimestamp = millis();
      lastAlertRetryTime = 0; // Trials timer reset
    //}
  }

  // If sending alert was unsuccessful
  if (!alertSent && !led.isLedBusy()) {
    // Max trial time has passed
    if (millis() - alertTimestamp >= ALERT_TIMEOUT) {
      Serial.println("Time has passed. Alert being omitted.");
      alertSent = true;
      currentError = ErrorCode::NONE;
    } else if (millis() - lastAlertRetryTime >= RETRY_INTERVAL || lastAlertRetryTime == 0) {
      lastAlertRetryTime = millis();
      Serial.println("Backend troubles. Sending query again.");

      // Try sending alert
      alertSent = BackendClient::sendAlert(statusCode);
      
      // When token has expired (401)
      if (!alertSent && statusCode == 401) {
        Serial.println("Token expired. Trying to authenticate.");
        deviceAuthenticated = BackendClient::authenticateDevice();

        // Try to send alert again
        if (deviceAuthenticated)
          alertSent = BackendClient::sendAlert(statusCode);
      }

      // Error update
      if (alertSent) {
        currentError = ErrorCode::NONE;
        Serial.println("Alert sent");
      } else {
        currentError = ErrorCode::HTTP_ERROR;
      }
    } 
  }
}