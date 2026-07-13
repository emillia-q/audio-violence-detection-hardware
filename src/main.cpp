#include <Arduino.h>
#include "Inmp441.h"
#include "AudioBuffer.h"
#include "MfccExtractor.h"
#include "CnnModel.h"
#include"secret.h"
#include"NvsManager.h"

// Pin configuration

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

// Global buffers (allocated in external PSRAM)
EXT_RAM_ATTR float modelInputBuffer[32000];
EXT_RAM_ATTR float modelFeaturesBuffer[feature_count]; // Ready features for CNN

void setup() {
  Serial.begin(115200);

  // NVS configuration
  if (!NvsManager::begin()) {
    Serial.println("NVS Error");
    while (1);
  }
  if (!NvsManager::hasDeviceSecret)
    NvsManager::saveDeviceSecret(DEVICE_SECRET);

  // Mic init
  if(mic.begin())
    Serial.println("INMP441 initialized successfully.");
  else {
    Serial.println("Failed to configure INMP441!");
    while(1);
  }

  // ESP-DSP MFCC init
  if(mfccExtr.begin())
    Serial.println("MFCC DSP Engine initialized successfully.");
  else {
    Serial.println("Failed to allocate memory for MFCC!");
    while(1);
  }

  // CNN model init
  if(cnnModel.begin())
    Serial.println("CNN Model loaded successfully.");
  else {
    Serial.println("Failed to load CNN model!");
    while(1);
  }
}

void loop() {
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
  }
}