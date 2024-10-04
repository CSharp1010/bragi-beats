// test_audioProcessing.c

#include "unity.h"
#include "../audio_processing/audio_processing.h"
#include "unity_internals.h"
#include <stddef.h>
#include <math.h>
#include <complex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 44100.0f
#define NUM_BINS 64
#define EPSILON 1e-6f
#define THRESHOLD 0.1f // Adjust as needed

AudioData testAudioData;
bool isPlaying = false;

void setUp(void) {

}

void tearDown(void) {

}

float* get_window_coefficients(void);
size_t* get_bit_reversal_indices(void);
float complex* get_twiddle_factors(void);


void test_init_audio_data(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    // Check if bufferIndex is initialized to 0
    TEST_ASSERT_EQUAL_size_t(0, audioData.bufferIndex);

    // Check if arrays are initialized to zero
    for (size_t i = 0; i < FFT_SIZE; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, audioData.in_raw[i]);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, audioData.in_win[i]);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, crealf(audioData.out_raw[i]));
        TEST_ASSERT_EQUAL_FLOAT(0.0f, cimagf(audioData.out_raw[i]));
        TEST_ASSERT_EQUAL_FLOAT(0.0f, audioData.out_log[i]);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, audioData.out_smooth[i]);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, audioData.out_phase[i]);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, audioData.out_power[i]);
    }
}

void test_fft(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    size_t n = FFT_SIZE;

    // Generate a simple sine wave
    generateSineWave(audioData.in_win, n, 1000.0f, SAMPLE_RATE);

    // Perform FFT
    fft(&audioData, n);

    // Check if the output has non-zero values (indicative of FFT processing)
    bool nonZeroFound = false;
    for (size_t i = 0; i < n; i++) {
        if (cabsf(audioData.out_raw[i]) > 0.0f) {
            nonZeroFound = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(nonZeroFound);
}

void test_apply_window_function(void) {
    float input[FFT_SIZE];
    float output[FFT_SIZE];

    // Initialize input with ones
    for (size_t i = 0; i < FFT_SIZE; i++) {
        input[i] = 1.0f;
    }

    apply_window_function(input, output, FFT_SIZE);

    // Check if output values are between 0 and 1 (since window functions scale the input)
    for (size_t i = 0; i < FFT_SIZE; i++) {
        TEST_ASSERT_TRUE(output[i] >= 0.0f && output[i] <= 1.0f);
    }
}

void test_ProcessFFT(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    // Fill input buffer with a test signal
    generateSineWave(audioData.in_win, FFT_SIZE, 1000.0f, SAMPLE_RATE);

    size_t n = ProcessFFT(&audioData);

    // Check if n equals NUM_BINS 
    TEST_ASSERT_EQUAL_size_t(NUM_BINS, n);

    // Verify that output arrays are populated
    for (size_t i = 0; i < n; i++) {
        TEST_ASSERT_TRUE(audioData.out_log[i] >= 0.0f);
        TEST_ASSERT_TRUE(audioData.out_smooth[i] >= 0.0f);
    }
}

void test_computePhase(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    size_t n = FFT_SIZE;

    // Perform FFT on a known signal
    generateSineWave(audioData.in_win, n, 1000.0f, SAMPLE_RATE);
    fft(&audioData, n);

    computePhase(&audioData, n);

    // Check if phase values are within the expected range
    for (size_t i = 0; i < n; i++) {
        TEST_ASSERT_TRUE(audioData.out_phase[i] >= -M_PI && audioData.out_phase[i] <= M_PI);
    }
}

void test_computePowerSpectrum(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    size_t n = FFT_SIZE;

    // Perform FFT on a known signal
    generateSineWave(audioData.in_win, n, 1000.0f, SAMPLE_RATE);
    fft(&audioData, n);

    computePowerSpectrum(&audioData, n);

    // Check if power values are non-negative
    for (size_t i = 0; i < n; i++) {
        TEST_ASSERT_TRUE(audioData.out_power[i] >= 0.0f);
    }
}

void test_detectPeaks(void) {
    AudioData audioData;
    init_audio_data(&audioData);
    bool peaks[FFT_SIZE];

    // Generate a signal with known peaks
    generateSineWave(audioData.in_win, FFT_SIZE, 1000.0f, SAMPLE_RATE);

    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    // Compute log of power spectrum
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        audioData.out_log[i] = log10f(audioData.out_power[i] + EPSILON);
    }

    detectPeaks(&audioData, FFT_SIZE, peaks);

    // Check if peaks are detected at expected frequencies
    size_t peakCount = 0;
    for (size_t i = 0; i < FFT_SIZE / 2; i++) {
        if (peaks[i]) {
            peakCount++;
        }
    }

    TEST_ASSERT_TRUE(peakCount > 0);
}

void test_applyBandpassFilter(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    // Generate a signal with multiple frequencies
    float frequencies[] = {500.0f, 1000.0f, 2000.0f};
    generateMultiSineWave(audioData.in_win, FFT_SIZE, frequencies, 3, SAMPLE_RATE);


    // Check if frequencies outside the band are attenuated
    fft(&audioData, FFT_SIZE);
    applyBandpassFilter(&audioData, FFT_SIZE, 800.0f, 1500.0f, SAMPLE_RATE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    // Expect peak around 1000 Hz and attenuation at 500 Hz and 2000 Hz
    size_t index500Hz = (size_t)(500.0f / SAMPLE_RATE * FFT_SIZE);
    size_t index1000Hz = (size_t)(1000.0f / SAMPLE_RATE * FFT_SIZE);
    size_t index2000Hz = (size_t)(2000.0f / SAMPLE_RATE * FFT_SIZE);

    // Ensure indices are within bounds
    if (index500Hz >= FFT_SIZE) index500Hz = FFT_SIZE - 1;
    if (index1000Hz >= FFT_SIZE) index1000Hz = FFT_SIZE - 1;
    if (index2000Hz >= FFT_SIZE) index2000Hz = FFT_SIZE - 1;

    TEST_ASSERT_TRUE(audioData.out_power[index1000Hz] > audioData.out_power[index500Hz]);
    TEST_ASSERT_TRUE(audioData.out_power[index1000Hz] > audioData.out_power[index2000Hz]);
}

void test_generateSineWave_zero_frequency(void) {
    float buffer[FFT_SIZE];
    generateSineWave(buffer, FFT_SIZE, 0.0f, SAMPLE_RATE);

    // The buffer should contain all zeros
    for (size_t i = 0; i < FFT_SIZE; i++) {
        TEST_ASSERT_FLOAT_WITHIN(EPSILON, 0.0f, buffer[i]);
    }
}

void test_generateSineWave_nyquist_frequency(void) {
    float buffer[FFT_SIZE];
    // Use a frequency slightly less than Nyquist to avoid sampling issues
    float nyquistFreq = (SAMPLE_RATE / 2.0f) - (SAMPLE_RATE / FFT_SIZE);
    generateSineWave(buffer, FFT_SIZE, nyquistFreq, SAMPLE_RATE);

    // Perform FFT and check for a peak near Nyquist frequency
    AudioData audioData;
    init_audio_data(&audioData);
    memcpy(audioData.in_win, buffer, sizeof(buffer));

    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    size_t index = FFT_SIZE / 2 - 1; // Index corresponding to nyquistFreq
    // Sum power around the expected index to account for spectral leakage
    float powerSum = 0.0f;
    for (size_t i = index - 1; i <= index + 1 && i < FFT_SIZE; i++) {
        powerSum += audioData.out_power[i];
    }

    char message[100];
    snprintf(message, sizeof(message), "Power at frequency %.2f Hz is below threshold", nyquistFreq);

    TEST_ASSERT_TRUE_MESSAGE(powerSum > THRESHOLD, message);
}

void test_generateWhiteNoise(void) {
    float buffer[FFT_SIZE];
    generateWhiteNoise(buffer, FFT_SIZE);

    // Check if buffer contains non-zero values
    bool nonZeroFound = false;
    for (size_t i = 0; i < FFT_SIZE; i++) {
        if (buffer[i] != 0.0f) {
            nonZeroFound = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(nonZeroFound);
}

void test_generateChirpSignal(void) {
    float buffer[FFT_SIZE];
    generateChirpSignal(buffer, FFT_SIZE, 500.0f, 2000.0f, SAMPLE_RATE);

    // Since it's a chirp, we expect varying frequencies
    // Perform FFT and check the spectrum spreads over frequencies
    AudioData audioData;
    init_audio_data(&audioData);
    memcpy(audioData.in_win, buffer, sizeof(buffer));
    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    // Check if multiple frequencies have significant power
    size_t count = 0;
    for (size_t i = 0; i < FFT_SIZE / 2; i++) {
        if (audioData.out_power[i] > THRESHOLD) {
            count++;
        }
    }
    TEST_ASSERT_TRUE(count > 1);
}

void test_generateMultiSineWave(void) {
    float buffer[FFT_SIZE];
    float frequencies[] = {500.0f, 1000.0f, 1500.0f};
    generateMultiSineWave(buffer, FFT_SIZE, frequencies, 3, SAMPLE_RATE);

    // Perform FFT and check peaks at specified frequencies
    AudioData audioData;
    init_audio_data(&audioData);
    memcpy(audioData.in_win, buffer, sizeof(buffer));
    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    for (size_t i = 0; i < 3; i++) {
        size_t index = (size_t)(frequencies[i] / SAMPLE_RATE * FFT_SIZE);
        TEST_ASSERT_TRUE(audioData.out_power[index] > THRESHOLD);
    }
}

void test_generateSineWave(void) {
    float buffer[FFT_SIZE];
    generateSineWave(buffer, FFT_SIZE, 1000.0f, SAMPLE_RATE);

    // Perform FFT and check for a peak at 1000 Hz
    AudioData audioData;
    init_audio_data(&audioData);
    memcpy(audioData.in_win, buffer, sizeof(buffer));
    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    size_t index = (size_t)(1000.0f / SAMPLE_RATE * FFT_SIZE);
    TEST_ASSERT_TRUE(audioData.out_power[index] > THRESHOLD);
}

void test_generateSineWave_negative_frequency(void) {
    float buffer[FFT_SIZE];
    generateSineWave(buffer, FFT_SIZE, -1000.0f, SAMPLE_RATE);

    // The function should handle negative frequencies appropriately
    // Depending on your implementation, you might expect an error or the absolute value
    // For this test, let's assume it uses the absolute value
    // Perform FFT and check for a peak at 1000 Hz
    AudioData audioData;
    init_audio_data(&audioData);
    memcpy(audioData.in_win, buffer, sizeof(buffer));
    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    size_t index = (size_t)(1000.0f / SAMPLE_RATE * FFT_SIZE);
    TEST_ASSERT_TRUE(audioData.out_power[index] > THRESHOLD);
}

void test_frequency_sweep(void) {
    float buffer[FFT_SIZE];
    AudioData audioData;

    // Test frequencies from 20 Hz to 20,000 Hz
    for (float freq = 20.0f; freq <= 20000.0f; freq *= 2.0f) {
        init_audio_data(&audioData);
        generateSineWave(buffer, FFT_SIZE, freq, SAMPLE_RATE);
        memcpy(audioData.in_win, buffer, sizeof(buffer));

        fft(&audioData, FFT_SIZE);
        computePowerSpectrum(&audioData, FFT_SIZE);

        size_t index = (size_t)(freq / SAMPLE_RATE * FFT_SIZE);
        if (index >= FFT_SIZE) index = FFT_SIZE - 1;

        char message[100];
        snprintf(message, sizeof(message), "Power at frequency %.2f Hz is below threshold", freq);

        TEST_ASSERT_TRUE_MESSAGE(audioData.out_power[index] > THRESHOLD, message);
    }
}

void test_fft_zero_length(void) {
    AudioData audioData;
    init_audio_data(&audioData);

    size_t n = 0; // Zero-length input

    // Attempt to perform FFT with zero-length input
    fft(&audioData, n);

    // Since n is zero, ensure that the function handled it gracefully
    // You might need to modify fft() to return an error code or handle this case
    // For now, we can check that no changes occurred to out_raw
    for (size_t i = 0; i < FFT_SIZE; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, crealf(audioData.out_raw[i]));
        TEST_ASSERT_EQUAL_FLOAT(0.0f, cimagf(audioData.out_raw[i]));
    }
}

void test_generateSineWave_max_frequency(void) {
    float buffer[FFT_SIZE];
    generateSineWave(buffer, FFT_SIZE, 20000.0f, SAMPLE_RATE);

    // Perform FFT and check for a peak at 20,000 Hz
    AudioData audioData;
    init_audio_data(&audioData);
    memcpy(audioData.in_win, buffer, sizeof(buffer));
    fft(&audioData, FFT_SIZE);
    computePowerSpectrum(&audioData, FFT_SIZE);

    size_t index = (size_t)(20000.0f / SAMPLE_RATE * FFT_SIZE);
    if (index >= FFT_SIZE) index = FFT_SIZE - 1;

    TEST_ASSERT_TRUE(audioData.out_power[index] > THRESHOLD);
}

void test_get_window_coefficients(void) {
    float *coefficients = get_window_coefficients();
    TEST_ASSERT_NOT_NULL(coefficients);

    // Check if coefficients are within valid range
    for (size_t i = 0; i < FFT_SIZE; i++) {
        TEST_ASSERT_TRUE(coefficients[i] >= 0.0f && coefficients[i] <= 1.0f);
    }
}

void test_get_bit_reversal_indices(void) {
    size_t *indices = get_bit_reversal_indices();
    TEST_ASSERT_NOT_NULL(indices);

    // Check if indices are within valid range
    for (size_t i = 0; i < FFT_SIZE; i++) {
        TEST_ASSERT_TRUE(indices[i] < FFT_SIZE);
    }
}

void test_get_twiddle_factors(void) {
    float complex *twiddles = get_twiddle_factors();
    TEST_ASSERT_NOT_NULL(twiddles);

    // Since twiddle factors are complex exponentials, their magnitude should be 1
    for (size_t i = 0; i < FFT_SIZE / 2; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, cabsf(twiddles[i]));
    }
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_init_audio_data);
    RUN_TEST(test_fft);
    RUN_TEST(test_apply_window_function);
    RUN_TEST(test_ProcessFFT);
    RUN_TEST(test_computePhase);
    RUN_TEST(test_computePowerSpectrum);
    RUN_TEST(test_detectPeaks);
    RUN_TEST(test_applyBandpassFilter);
    RUN_TEST(test_generateWhiteNoise);
    RUN_TEST(test_generateChirpSignal);
    RUN_TEST(test_generateMultiSineWave);
    RUN_TEST(test_generateSineWave);
    RUN_TEST(test_get_window_coefficients);
    RUN_TEST(test_get_bit_reversal_indices);
    RUN_TEST(test_get_twiddle_factors);
    RUN_TEST(test_fft_zero_length);
    RUN_TEST(test_generateSineWave_zero_frequency);
    RUN_TEST(test_generateSineWave_nyquist_frequency);
    RUN_TEST(test_fft_zero_length);
    RUN_TEST(test_generateSineWave_max_frequency);
    RUN_TEST(test_generateSineWave_negative_frequency);
    RUN_TEST(test_frequency_sweep);

    return UNITY_END();
}
