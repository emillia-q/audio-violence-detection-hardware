#include "AudioBuffer.h"

AudioBuffer::AudioBuffer()
{
    ringBuffer = (int16_t*)malloc(WINDOW_SIZE * sizeof(int16_t));   // Allocate memory on heap
    memset(ringBuffer, 0, WINDOW_SIZE * sizeof(int16_t));   // Clean up
    writePointer = 0;
    samplesSinceLastStep = 0;
}

AudioBuffer::~AudioBuffer()
{
    free(ringBuffer);
}

void AudioBuffer::addSample(int16_t sample)
{
    ringBuffer[writePointer] = sample;
    writePointer = (writePointer + 1) % WINDOW_SIZE;
    samplesSinceLastStep++;
}

void AudioBuffer::extractAndNormalizeWindow(float *outputBuffer)
{
    int readPointer = writePointer;
    float maxAbs = 0.0f;

    for (int i = 0; i < WINDOW_SIZE; i++) {
        outputBuffer[i] = (float)ringBuffer[readPointer];
        readPointer = (readPointer + 1) % WINDOW_SIZE; 

        float calcAbs = fabs(outputBuffer[i]);
        if (calcAbs > maxAbs)
            maxAbs = calcAbs;
    }

    // Normalize [-1.0, 1.0]
    // Perform division only if the sound is not perfect silence - unlikely to happen
    if (maxAbs > 0.0001f) {
        for (int i = 0; i < WINDOW_SIZE; i++)
            outputBuffer[i] = outputBuffer[i] / maxAbs;
    }

}

bool AudioBuffer::isWindowReady()
{
    if (samplesSinceLastStep >= SAMPLE_RATE) {
        samplesSinceLastStep = 0;
        return true;
    }
    return false;
}
