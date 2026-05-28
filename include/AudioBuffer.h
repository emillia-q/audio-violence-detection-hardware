#pragma once

#include <Arduino.h>

class AudioBuffer{
    static const int SAMPLE_RATE = 16000;           // Adjusted to the model (16kHz)
    static const int WINDOW_SIZE = SAMPLE_RATE * 2; // 32000 samples (2s)
    static const int STEP_SIZE = SAMPLE_RATE;       // 16000 samples (1s)

    int16_t* ringBuffer;    // Dynamic heap buffer (32000 * 2B = 64 KB)
    int writePointer;
    int samplesSinceLastStep;
public:
    AudioBuffer();
    ~AudioBuffer();

    void addSample(int16_t sample);
    void extractAndNormalizeWindow(float* outputBuffer);
    bool isWindowReady();
};
