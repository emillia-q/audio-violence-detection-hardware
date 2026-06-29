# pragma once

#include <Arduino.h>

// Model
#include "model_data.h"

// TensorFlow
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

class CnnModel {
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* model_input = nullptr;
    TfLiteTensor* model_output = nullptr;
    // Tensor Arena - RAM area where TF performs the mathematical operations of the network
    uint8_t* tensor_arena = nullptr;
    const int kTensorArenaSize = 128 * 1024; // 128 KB

    tflite::AllOpsResolver resolver;
    tflite::MicroErrorReporter micro_error_reporter;

public:
    CnnModel();
    ~CnnModel();

    bool begin();
    void prediction(const float* modelFeaturesBuffer);
};