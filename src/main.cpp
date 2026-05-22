#include <Arduino.h>
#include <driver/i2s.h>

// Pin configuration
#define I2S_WS   15   // Word Select
#define I2S_SD   13   // Serial Data
#define I2S_SCK  4   // Serial Clock

// Use first available I2S port
#define I2S_PORT I2S_NUM_0


void setup() {
  Serial.begin(115200);
}

void loop() {
  
}