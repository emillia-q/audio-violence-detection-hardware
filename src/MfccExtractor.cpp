#include "MfccExtractor.h"

MfccExtractor::MfccExtractor() : fft_complex_buf(nullptr), window_coeffs(nullptr), power_spectrum(nullptr),
                                 mel_weights(nullptr), mel_spectrogram(nullptr), dct_matrix(nullptr)
{
}

MfccExtractor::~MfccExtractor()
{
    free(fft_complex_buf);
    free(window_coeffs);
    free(power_spectrum);
    free(mel_weights);
    free(mel_spectrogram);
    free(dct_matrix);
}

bool MfccExtractor::begin()
{
    // Heap memory allocation
    // SRAM
    fft_complex_buf = (float*)heap_caps_malloc(2 * N_FFT * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
    window_coeffs = (float*)heap_caps_malloc(N_FFT * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
    power_spectrum = (float*)heap_caps_malloc((N_FFT / 2 + 1) * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);

    // PSRAM
    mel_weights = (float*)heap_caps_malloc((N_FFT / 2 + 1) * N_MELS * sizeof(float), MALLOC_CAP_SPIRAM);
    mel_spectrogram = (float*)heap_caps_malloc(N_FRAMES * N_MELS * sizeof(float), MALLOC_CAP_SPIRAM);
    dct_matrix = (float*)heap_caps_malloc(N_MFCC * N_MELS * sizeof(float), MALLOC_CAP_SPIRAM);
    
    if (!fft_complex_buf || !window_coeffs || !power_spectrum || !mel_weights || !mel_spectrogram || !dct_matrix)
        return false; // Lack of RAM memory

    // ESP-DSP library initialization for FFT
    esp_err_t err = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (err != ESP_OK)
        return false; 

        // Periodic Hann window to match librosa/scipy get_window('hann', N, fftbins=True)
    for (int i = 0; i < N_FFT; i++)
        window_coeffs[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / N_FFT));

    // Pre-calibration of Mel and DCT filter matrices
    initMelFilterbank();
    initDctMatrix();

    return true;
}

void MfccExtractor::compute(const float *audio, float *mfcc_out)
{
    // Build the power mel spectrogram for all 63 frames
    for (int f = 0; f < N_FRAMES; f++) {
        extractFrame(audio, f);
        applyWindowAndFft();
        computePowerSpectrum();
        applyMelFilterbank(&mel_spectrogram[f * N_MELS]);
    }

    // Convert the WHOLE spectrogram to dB
    powerToDb();

    // DCT-II per frame -> [63][13] features
    for (int f = 0; f < N_FRAMES; f++)
    computeDct(&mel_spectrogram[f* N_MELS], &mfcc_out[f* N_MFCC]);

}

void MfccExtractor::initMelFilterbank()
{
    // Convert Hz <-> Mel using the Slaney scale (librosa default, htk=False):
    // linear below 1000 Hz, logarithmic above.
    const float f_sp = 200.0f / 3.0f;             // 66.667 Hz per mel in the linear region
    const float min_log_hz = 1000.0f;
    const float min_log_mel = min_log_hz / f_sp;  // 15.0
    const float logstep = logf(6.4f) / 27.0f;

    auto hz_to_mel = [&](float hz) -> float {
        if (hz < min_log_hz)
            return hz / f_sp;
        return min_log_mel + logf(hz / min_log_hz) / logstep;
    };
    auto mel_to_hz = [&](float mel) -> float {
        if (mel < min_log_mel)
            return f_sp * mel;
        return min_log_hz * expf(logstep * (mel - min_log_mel));
    };

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
    int start_sample = frame_idx * HOP_LENGTH - (N_FFT / 2);

    for (int i = 0; i < N_FFT; i++) {
        int idx = start_sample + i;

        // librosa center=True pads the signal with mode='reflect' (no edge repetition).
        // Single reflection is enough here because N_FFT/2 (1024) << N_SAMPLES (32000).
        if (idx < 0)
            idx = -idx;
        else if (idx >= N_SAMPLES)
            idx = 2 * (N_SAMPLES - 1) - idx;

        // Complex format for ESP-DSP: [Re0, Im0, Re1, Im1...]
        fft_complex_buf[2 *i] = audio[idx]; // Re
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

void MfccExtractor::applyMelFilterbank(float* out)
{
    memset(out, 0, N_MELS * sizeof(float));
    int fft_bins = N_FFT / 2 + 1;

    // Multiplying the power spectrum by triangular mel filter banks
    for (int bin = 0; bin < fft_bins; bin++) {
        float p_val = power_spectrum[bin];
        if (p_val == 0.0f)
            continue;
        
        for (int m = 0; m < N_MELS; m++) 
            out[m] += p_val * mel_weights[bin * N_MELS + m];
    }
}

void MfccExtractor::powerToDb()
{
    // Defaults used by librosa.feature.mfcc:
    // ref=1.0, amin=1e-10, top_db=80.0
    // It must run after all frames are ready
    const int total = N_FRAMES * N_MELS;
    const float amin = 1e-10f;
    const float top_db = 80.0f;

    float max_db = -1e30f;
    for (int i = 0; i < total; i++) {
        float v = (mel_spectrogram[i] < amin) ? amin : mel_spectrogram[i];
        float db = 10.0f * log10f(v); // ref=1.0 -> minus 10*log10(1) = 0
        mel_spectrogram[i] = db;
        if (db > max_db)
            max_db = db;
    }

    float floor_db = max_db - top_db;
    for (int i = 0; i < total; i++) {
        if (mel_spectrogram[i] < floor_db)
            mel_spectrogram[i] = floor_db;
    }
}

void MfccExtractor::computeDct(const float *log_mel, float *frame_out)
{
    // Multiply the log_mel vector by the DCT-II matrix to obtain 13 MFCC features
    for (int i = 0; i < N_MFCC; i++) {
        float sum = 0.0f;
        for (int j = 0; j < N_MELS; j++) 
            sum += log_mel[j] * dct_matrix[i * N_MELS + j];
        frame_out[i] = sum;
    }
}