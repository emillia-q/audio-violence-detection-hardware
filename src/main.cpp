#include <Arduino.h>
#include "Inmp441.h"

// Pin configuration

// INMP441
#define MIC_WS   15         // Word Select
#define MIC_SD   13         // Serial Data
#define MIC_SCK  4          // Serial Clock
#define I2S_PORT I2S_NUM_0  // Use first available I2S port

// Object configuration

// INMP441
Inmp441 mic(MIC_WS, MIC_SD, MIC_SCK, I2S_PORT);

void setup() {
  Serial.begin(115200);

  if(mic.begin())
    Serial.println("INMP441 initialized successfully.");
  else {
    Serial.println("Failed to configure INMP441!");
    while(1);
  }

}

void loop() {
  
}