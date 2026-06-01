#include <Arduino.h>
#include "Inmp441.h"
#include "AudioBuffer.h"
#include "MfccExtractor.h"
#include "golden_input.h"

// Model
#include "model_data.h"

// TensorFlow
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

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
EXT_RAM_ATTR float modelInputBuffer[32000];
EXT_RAM_ATTR float modelFeaturesBuffer[63 * 13]; // Ready features for CNN

// Tflite config
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  TfLiteTensor* model_output = nullptr;

  // Tensor Arena - RAM area where TF performs the mathematical operations of the network
  const int kTensorArenaSize = 128 * 1024; // 128 KB
  uint8_t* tensor_arena = nullptr;
}

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

  // ---------------- TEST -------------------
  Serial.println("--- START TEST ---");
  
  // Normalize audio data
  // Same logic as in AudioBuffer but without using ringBuffer
  for(int i = 0; i < 32000; i++) {
      modelInputBuffer[i] = golden_input[i];
  }

  float maxAbs = 0.0f;
  for (int i = 0; i < 32000; i++) {
      float calcAbs = fabs(modelInputBuffer[i]);
      if (calcAbs > maxAbs) maxAbs = calcAbs;
  }

  // Normalize
  if (maxAbs > 0.0001f) {
      for (int i = 0; i < 32000; i++) {
          modelInputBuffer[i] = modelInputBuffer[i] / maxAbs;
      }
  }

  Serial.println("--- FIRST 10 SAMPLES ---");
  for (int i = 0; i < 10; i++) {
      Serial.printf("[%d]: %.8f\n", i, modelInputBuffer[i]);
  }

  // Compute mfcc
  Serial.println("MFCC TEST");
  mfccExtr.compute(modelInputBuffer, modelFeaturesBuffer);
    
  Serial.println("13 MFCC:");
  for (int i = 0; i < 819; i++) {
      Serial.printf("%.8f\n", modelFeaturesBuffer[i]);
  }
  Serial.println("--------------------------");

  while(1);

  // ---------------- END TEST -------------------

  // Tflite init
  // Allocate memory in PSRAM
  tensor_arena = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);
  if (tensor_arena == nullptr) {
    Serial.println("Failed to allocate Tensor Arena in PSRAM!");
    while(1);
  }

  // Set up logging
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model version does not match Schema.");
    while(1);
  }

  // TODO: Load only data used in model training (Conv2D, Dense, MaxPool etc.)
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed!");
    while(1);
  }

  // Assign model input and output buffers to pointers
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);
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

    // MFCC extraction
    mfccExtr.compute(modelInputBuffer, modelFeaturesBuffer);

    // Copy calculated MFCC to tensor input
    // 63 * 13 = 819
    for (int i = 0; i < 63 * 13; i++) {
        model_input->data.f[i] = modelFeaturesBuffer[i];
    }

    // Invoke CNN
    unsigned long inferenceStart = millis();
    TfLiteStatus invoke_status = interpreter->Invoke();
    unsigned long inferenceEnd = millis();

    if (invoke_status != kTfLiteOk)
        error_reporter->Report("Invoke failed!");

    Serial.printf("AI Inference successful in %lu ms!\n", inferenceEnd - inferenceStart);

    Serial.println("--- PRED ---");
    Serial.printf("Ambient: %.4f\n", model_output->data.f[0]);
    Serial.printf("Speech: %.4f\n", model_output->data.f[1]);
    Serial.printf("Violence: %.4f\n", model_output->data.f[2]);

    Serial.println("-----------------------------");
  }
}