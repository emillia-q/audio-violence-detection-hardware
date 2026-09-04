#include <Arduino.h>
#include "Inmp441.h"
#include "AudioBuffer.h"
#include "MfccExtractor.h"
#include "CnnModel.h"
#include"secret.h"
#include"NvsManager.h"
#include"WifiPortal.h"
#include"BackendClient.h"
#include "Led.h"
#include "Button.h"

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

// Model configuration
constexpr size_t feature_count = 63 * 13;

// Object instances
Inmp441 mic(MIC_WS, MIC_SD, MIC_SCK, I2S_PORT);
AudioBuffer audioBuffer;
MfccExtractor mfccExtr;
CnnModel cnnModel;
ErrorCode currentError = ErrorCode::NONE;
Led led(RED_LED);
ButtonEvent buttonEvent = ButtonEvent::NONE;
Button button(BUTTON_PIN);

// Global buffers (allocated in external PSRAM)
EXT_RAM_ATTR float modelInputBuffer[32000];
EXT_RAM_ATTR float modelFeaturesBuffer[feature_count]; // Ready features for CNN

// Program variables
bool isConfigMode = false;
int statusCode = 0;
bool alertSent = false;
bool deviceAuthenticated = false;

void setup() {
  Serial.begin(115200);

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
    // Connect to WiFi
    while (!WifiPortal::connectToSavedWifi()) 
        Serial.println("Trying to connect again");
        
     // Check if device is already active & assigned to a user in database
    if (!NvsManager::isActivated()) {
      if (!BackendClient::activateDevice()) {
        Serial.println("Device activation failed");
        while (1);
      }
    }

    // Wifi credentials saved & device assigned to a user
    // Mic init
    if(mic.begin())
      Serial.println("INMP441 initialized successfully");
    else {
      Serial.println("Failed to configure INMP441");
      currentError = ErrorCode::HARDWARE_ERROR;
    }

    // ESP-DSP MFCC init
    if(mfccExtr.begin())
      Serial.println("MFCC DSP Engine initialized successfully");
    else {
      Serial.println("Failed to allocate memory for MFCC");
      currentError = ErrorCode::HARDWARE_ERROR;
    }

    // CNN model init
    if(cnnModel.begin())
      Serial.println("CNN Model loaded successfully.");
    else {
      Serial.println("Failed to load CNN model!");
      currentError = ErrorCode::HARDWARE_ERROR;
    }
  }
}

void loop() {
  // Checks if user has connected to hotspot
  if (isConfigMode) {
    WifiPortal::handleClient();
    delay(1);
    return;
  }

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

  int16_t sample_buffer[BUFFER_LEN];
  int samples_read = mic.readSamples(sample_buffer, BUFFER_LEN);
  for (int i = 0; i < samples_read; i++)
    audioBuffer.addSample(sample_buffer[i]);

  if (audioBuffer.isWindowReady()) {
    audioBuffer.extractAndNormalizeWindow(modelInputBuffer);

    // MFCC extraction
    mfccExtr.compute(modelInputBuffer, modelFeaturesBuffer);

    // Model prediction
    cnnModel.prediction(modelFeaturesBuffer, feature_count);

    // Check if violence was detected & send alert
    if (cnnModel.violenceDetected())
      alertSent = BackendClient::sendAlert(statusCode);
  }

  // If sending alert was unsuccessful & response status was unauthorized
  if (statusCode == 401 && !alertSent) {
    // Authorize & try again
    deviceAuthenticated = BackendClient::authenticateDevice();
    alertSent = BackendClient::sendAlert(statusCode);
  }

  // Other backend troubles
  if (!deviceAuthenticated && !alertSent) {
    // TODO: invalid request payload or troubles with authorization
  }

  if (deviceAuthenticated && !alertSent) {
    // TODO: wrong token format, wrong role, device or user not found -> display it to the user
  }
}