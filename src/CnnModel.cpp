#include "CnnModel.h"

CnnModel::CnnModel()
{
}

CnnModel::~CnnModel()
{
    free(tensor_arena);
}

bool CnnModel::begin()
{
    // Tflite init
    // Allocate memory in PSRAM
    tensor_arena = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);
    if (tensor_arena == nullptr) {
        Serial.println("Failed to allocate Tensor Arena in PSRAM!");
        return false;
    }

    // Set up logging
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // Map the model into a usable data structure
    model = tflite::GetModel(model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("Model version does not match Schema.");
        return false;
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
        return false;
    }

    // Assign model input and output buffers to pointers
    model_input = interpreter->input(0);
    model_output = interpreter->output(0);
    return true;
}
