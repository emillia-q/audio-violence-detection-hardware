#pragma once

#include <Arduino.h>
#include <esp_dsp.h>

class MfccExtractor {
    // Signal processing constants (compatible with Librosa default parameters)
    const int N_FFT = 2048;
    const int HOP_LENGTH = 512;
    const int N_MELS = 128;
    const int N_MFCC = 13;
    const int N_FRAMES = 63;
    const float SAMPLE_RATE = 16000.0f;

    // Buffers allocated once at startup
    float* fft_complex_buf;    // 2 * N_FFT = 4096 (Re, Im, Re, Im...)
    float* window_coeffs;      // N_FFT = 2048 (Hann window coefficients)
    float* power_spectrum;     // N_FFT / 2 + 1 = 1025 (frequency bars)
    float* mel_weights;        // 1025 * 128 = 131200 floatów (filter matrix)
    float* mel_energies;       // N_MELS = 128 (energy on the Mel scale)
    float* dct_matrix;         // 13 * 128 = 1664 floatów (DCT-II matrix)

    // Functions implementing DSP pipeline
    void initMelFilterbank();
    void initDctMatrix();
    void extractFrame(const float* audio, int frame_idx);
    void applyWindowAndFft();
    void computePowerSpectrum();
    void applyMelFilterbank();
    void logTransform();
    void computeDct(float* frame_out);
public:
    MfccExtractor();
    ~MfccExtractor();

    bool begin();
    // Takes a pointer to a normalized 32000 float sample from AudioBuffer 
    // Writes the result to a flat array of size 63 * 13 = 819 floats
    void compute(const float* audio, float* mfcc_out);
};