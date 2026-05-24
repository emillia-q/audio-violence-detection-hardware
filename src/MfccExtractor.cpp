#include "MfccExtractor.h"

MfccExtractor::MfccExtractor() : fft_complex_buf(nullptr), window_coeffs(nullptr), power_spectrum(nullptr),
    mel_weights(nullptr), mel_energies(nullptr), dct_matrix(nullptr)
{
}

MfccExtractor::~MfccExtractor()
{
    free(fft_complex_buf);
    free(window_coeffs);
    free(power_spectrum);
    free(mel_weights);
    free(mel_energies);
    free(dct_matrix);
}

bool MfccExtractor::begin()
{
    // Heap memory allocation
    fft_complex_buf = (float*)malloc(2 * N_FFT * sizeof(float));
    window_coeffs = (float*)malloc(N_FFT * sizeof(float));
    power_spectrum = (float*)malloc((N_FFT / 2 + 1) * sizeof(float));
    mel_weights = (float*)malloc((N_FFT / 2 + 1) * N_MELS * sizeof(float));
    mel_energies = (float*)malloc(N_MELS * sizeof(float));
    dct_matrix = (float*)malloc(N_MFCC * N_MELS * sizeof(float));
    
    if (!fft_complex_buf || !window_coeffs || !power_spectrum || !mel_weights || !mel_energies || !dct_matrix)
        return false; // Lack of RAM memory

    // ESP-DSP library initialization for FFT
    esp_err_t err = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (err != ESP_OK)
        return false; 

    return true;
}
