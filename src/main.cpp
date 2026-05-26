#include <Arduino.h>
#include "Inmp441.h"
#include "AudioBuffer.h"
#include "MfccExtractor.h"

// Pin configuration

// INMP441
#define MIC_WS   15         // Word Select
#define MIC_SD   13         // Serial Data
#define MIC_SCK  4          // Serial Clock
#define I2S_PORT I2S_NUM_0  // Use first available I2S port
#define BUFFER_LEN 256

// Object configuration
Inmp441 mic(MIC_WS, MIC_SD, MIC_SCK, I2S_PORT);
AudioBuffer audioBuffer;
MfccExtractor mfccExtr;

// Global buffers
float modelInputBuffer[32000];
float modelFeaturesBuffer[63 * 13]; // Ready features for CNN

void setup() {
  Serial.begin(115200);

  if(mic.begin())
    Serial.println("INMP441 initialized successfully.");
  else {
    Serial.println("Failed to configure INMP441!");
    while(1);
  }

  if(mfccExtr.begin())
    Serial.println("MFCC DSP Engine initialized successfully.");
  else {
    Serial.println("Failed to allocate memory for MFCC!");
    while(1);
  }
}

void loop() {
  int32_t raw_hardware_buff[BUFFER_LEN];
  int samples_read = mic.readRawData(raw_hardware_buff, BUFFER_LEN);
  for (int i = 0; i < samples_read; i++) {
    // Shift right by 8 to drop empty low bits, then mask to keep clean 24-bit audio
    int32_t v = (raw_hardware_buff[i] >> 8) & 0xFFFFFF;
    // If the 24th bit is 1 (negative), sign-extend the top 8 bits to 0xFF
    if (v & 0x800000) 
      v |= 0xFF000000;
    
    int16_t sample = v >> 8; 
    audioBuffer.addSample(sample);
  }

  if (audioBuffer.isWindowReady()) {
    audioBuffer.extractAndNormalizeWindow(modelInputBuffer);

    unsigned long startTime = millis();
    mfccExtr.compute(modelInputBuffer, modelFeaturesBuffer);
    unsigned long endTime = millis();

    Serial.printf("MFCC computed successfully in %lu ms!\n", endTime - startTime);
    Serial.printf("Snapshot of Frame 0, Coeff 0: %.4f\n", modelFeaturesBuffer[0]);
  }
}