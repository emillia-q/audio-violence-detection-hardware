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

    // Pre-calibration of Mel and DCT filter matrices
    initMelFilterbank();
    initDctMatrix();

    return true;
}

void MfccExtractor::compute(const float *audio, float *mfcc_out)
{
    // Sliding window cutting 2 seconds of audio per 63 frames
    for (int f = 0; f < N_FRAMES; f++) {
        extractFrame(audio, f);
        applyWindowAndFft();
        computePowerSpectrum();
        applyMelFilterbank();
        logTransform();

        // Save frame result directly to the target output spectrogram [63][13]
        computeDct(&mfcc_out[f * N_MFCC]);
    }
}

void MfccExtractor::initMelFilterbank()
{
    // Convert Hz frequency to Mel scale (Librosa Slaney formula)
    auto hz_to_mel = [](float hz) {return 3.0f * logf(1.0f +hz / 700.0f);};
    auto mel_to_hz = [](float mel) {return 700.0f * (expf(mel / 3.0f))-1.0f;};

    float min_mel = hz_to_mel(0.0f);
    float max_mel = hz_to_mel(SAMPLE_RATE / 2.0f);

    // Generating triangular filter mesh points
    float mel_pts[N_MELS + 2];
    for (int i = 0; i < N_MELS + 2; i++) {
        float m = min_mel + i * (max_mel - min_mel) / (N_MELS +1);
        mel_pts[i] = mel_to_hz(m);
    } 

    int fft_bins = N_FFT / 2 + 1;
    memset(mel_weights, 0, fft_bins * N_MELS * sizeof(float));

    // Building triangular mel filter weights
    for (int m = 0; m < N_MELS; m++) {
        float f_low = mel_pts[m];
        float f_cent = mel_pts[m+1];
        float f_high = mel_pts[m+2];

        for (int bin = 0; bin < fft_bins; bin++) {
            float freq = bin * SAMPLE_RATE / N_FFT;
            float weight = 0.0f;

            if (freq >= f_low && freq <= f_cent) 
                weight = (freq - f_low) / (f_cent - f_low);
            else if (freq >= f_cent && freq <= f_high)
                weight = (f_high - freq) / (f_high - f_cent);

            // Slaney-style normalization - librosa
            float norm = 2.0f / (f_high - f_low);
            mel_weights[bin * N_MELS + m] = weight * norm;
        }
    }
}

void MfccExtractor::initDctMatrix()
{
    // Generating weights for Discrete Cosine Transform type II
    for (int i = 0; i < N_MFCC; i++) {
        float scale = (i==0) ? sqrtf(1.0f / N_MELS) : sqrtf(2.0f / N_MELS);
        for (int j = 0; j < N_MELS; j++) 
            dct_matrix[i * N_MELS + j] = scale * cosf((M_PI * i * (2 * j + 1)) / (2.0f * N_MELS));
    }
}

void MfccExtractor::extractFrame(const float *audio, int frame_idx)
{
    int start_sample = frame_idx * HOP_LENGTH;

    for (int i = 0; i < N_FFT; i++) {
        int audio_idx = start_sample + i;
        // Complex format for ESP-DSP: [Re0, Im0, Re1, Im1...]
        if (audio_idx < 32000) 
            fft_complex_buf[2 *i] = audio[audio_idx]; // Re
        else
            fft_complex_buf[2*i] = 0.0f; // Zero padding at the end of the timeline
        
        fft_complex_buf[2*i + 1] = 0.0f; // The imaginary part always starts from zero
    }
}

void MfccExtractor::applyWindowAndFft()
{
    // Use ESP-DSP vector multiplication to overlay the Hann window
    for (int i = 0; i < N_FFT; i++) 
        fft_complex_buf[2*i] *= window_coeffs[i];

    // Fourier transform
    dsps_fft2r_fc32(fft_complex_buf, N_FFT);
    // Binary sorting of results - required by the ESP-DSP algorithm
    dsps_bit_rev_fc32(fft_complex_buf, N_FFT);
}

void MfccExtractor::computePowerSpectrum()
{
    int fft_bins = N_FFT / 2 + 1;
    // Extract the amplitude (Power Spectrum) from complex numbers: Re^2 + Im^2
    for (int i = 0; i < fft_bins; i++) {
        float re = fft_complex_buf[2*i];
        float im = fft_complex_buf[2*i + 1];
        power_spectrum[i] = (re * re) + (im * im);
    }
}

void MfccExtractor::applyMelFilterbank()
{
    memset(mel_energies, 0, N_MELS * sizeof(float));
    int fft_bins = N_FFT / 2 + 1;

    // Multiplying the power spectrum by triangular mel filter banks
    for (int bin = 0; bin < fft_bins; bin++) {
        float p_val = power_spectrum[bin];
        if (p_val == 0.0f)
            continue;
        
        for (int m = 0; m < N_MELS; m++) 
            mel_energies[m] += p_val * mel_weights[bin * N_MELS + m];
    }
}

void MfccExtractor::logTransform()
{
    // Equivalent to librosa.power_to_db (convert energy to dB scale)
    for (int m = 0; m < N_MELS; m++) {
        // Protect against log(0) with a small epsilon (1e-5f)
        float val = (mel_energies[m] < 1e-5f) ? 1e-5f : mel_energies[m];
        mel_energies[m] = 10.0f * log10f(val);
    }
}

void MfccExtractor::computeDct(float *frame_out)
{
    // Multiply the log_mel vector by the DCT-II matrix to obtain 13 MFCC features
    for (int i = 0; i < N_MFCC; i++) {
        float sum = 0.0f;
        for (int j = 0; j < N_MELS; j++) 
            sum += mel_energies[j] * dct_matrix[i * N_MELS + j];
        frame_out[i] = sum;
    }
}