#include "audioProcessing.h"
#include "../infrastructure/config.h"
#include <complex.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <fftw3.h>

float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
fftwf_complex out_raw[FFT_SIZE];
float out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];
float out_phase[FFT_SIZE];
float out_power[FFT_SIZE];

/*
void fft(float in[], size_t stride, float complex out[], size_t n) {
    // there has to be elements in the array
    assert(n > 0);

    if (n == 1) {
        // base case: single element FFT is the element itself
        out[0] = in[0];
        return;
    }

    // process first half
    fft(in, stride*2, out, n/2);
    // process second half
    fft(in + stride, stride*2, out + n/2, n/2);

    // combine first and second half
    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k / n;

        float complex v = cexp(-2 * I * PI * t) * out[k + n / 2];

        float complex e = out[k];

        out[k] = e + v;

        out[k + n / 2] = e - v;
    }
}

// compute the amplitudes from complex numbers
float amp(float complex z) {
    float magnitude = cabs(z);
    // logarithmic scaling
    return logf(1 + magnitude);
}

// process each incoming frame to prepare a buffer for Raylib audio functions
void callback(void *bufferData, unsigned int frames) {
    if (frames > FFT_SIZE) frames = FFT_SIZE;

    // cast to a 2D array of floats
    float (*fs)[2] = bufferData;

    // sliding window across input buffer and insert new samples
    for (size_t i = 0; i < frames; ++i) {
        memmove(in_raw, in_raw + 1, (FFT_SIZE-1)*sizeof(in_raw[0]));
        in_raw[FFT_SIZE-1] = fs[i][0];
    }
}

// generate logarithmic and smoothed objects
size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[]) {

    float dt = GetFrameTime();

    // apply a Hann window to raw input data
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i/(FFT_SIZE-1);
        float hann = 0.5 - 0.5*cosf(2*PI*t);
        in_win[i] = in_raw[i]*hann;
    }

    // fft again of windowed data
    fft(in_win, 1, out_raw, FFT_SIZE);

    float step = 1.07;
    float lowf = 1.0f;
    size_t numberFftBins = 0;
    float max_amp = 1.0f;

    // loop over fft bins and calculate amplitude
    for (float f = lowf; (size_t) f < FFT_SIZE/2; f = f + ceilf(f * (step - 1))) {
        float f1 = ceilf(f*step);
        float a = 0.0f;
        for (size_t q = (size_t) f; q < FFT_SIZE/2 && q < (size_t) f1; ++q) {
            float b = amp(out_raw[q]);
            if (b > a) a = b;
        }

        if (max_amp < a) max_amp = a;
        out_log[numberFftBins++] = a;
    }

    for (size_t i = 0; i < numberFftBins; ++i) {
        out_log[i] /= max_amp;
    }

    float smoothness = 7.5f;
    for (size_t i = 0; i < numberFftBins; ++i) {
        out_smooth[i] += (out_log[i] - out_smooth[i]) * smoothness * dt;

    }

    return numberFftBins;
}
*/

fftwf_plan plan;

void initializeFFT() {
    plan = fftwf_plan_dft_r2c_1d(FFT_SIZE, in_win, out_raw, FFTW_ESTIMATE);
}

void cleanupFFT() {
    fftwf_destroy_plan(plan);
}

// Compute the amplitudes from complex numbers
float amp(fftwf_complex z) {
    float magnitude = cabsf(z); // Single-precision complex magnitude
    return logf(1 + magnitude);
}

// Compute the phase from complex numbers
float phase(fftwf_complex z) {
    return atan2f(cimagf(z), crealf(z));
}

// Compute the power spectral density
float power(fftwf_complex z) {
    return crealf(z) * crealf(z) + cimagf(z) * cimagf(z);
}

// Process each incoming frame to prepare a buffer for Raylib audio functions
void callback(void *bufferData, unsigned int frames) {
    if (frames > FFT_SIZE) frames = FFT_SIZE;

    // Cast to a 2D array of floats
    float (*fs)[2] = bufferData;

    // Sliding window across input buffer and insert new samples
    for (size_t i = 0; i < frames; ++i) {
        memmove(in_raw, in_raw + 1, (FFT_SIZE-1)*sizeof(in_raw[0]));
        in_raw[FFT_SIZE-1] = fs[i][0];
    }
}

// Generate logarithmic, smoothed, phase, and power objects
size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[], float out_phase[], float out_power[]) {
    float dt = GetFrameTime();

    // Apply a Hann window to raw input data
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i / (FFT_SIZE - 1);
        float hann = 0.5 - 0.5 * cosf(2 * PI * t);
        in_win[i] = in_raw[i] * hann;
    }

    // Perform FFT on windowed data
    fftwf_execute(plan);

    float step = 1.07;
    float lowf = 1.0f;
    size_t numberFftBins = 0;
    float max_amp = 1.0f;
    float max_power = 1.0f;

    // Loop over FFT bins and calculate amplitude, phase, and power
    for (float f = lowf; (size_t)f < FFT_SIZE / 2; f = f + ceilf(f * (step - 1))) {
        float f1 = ceilf(f * step);
        float a = 0.0f;
        size_t max_idx = 0;
        for (size_t q = (size_t)f; q < FFT_SIZE / 2 && q < (size_t)f1; ++q) {
            float b = amp(out_raw[q]);
            if (b > a) {
                a = b;
                max_idx = q;
            }
        }

        if (max_amp < a) max_amp = a;
        out_log[numberFftBins] = a;
        out_phase[numberFftBins] = phase(out_raw[max_idx]);
        out_power[numberFftBins] = power(out_raw[max_idx]);
        if (out_power[numberFftBins] > max_power) {
            max_power = out_power[numberFftBins];
        }
        numberFftBins++;
    }

    for (size_t i = 0; i < numberFftBins; ++i) {
        out_log[i] /= max_amp;
        out_power[i] /= max_power;  // Normalize power values
    }

    float smoothness = 7.5f;
    for (size_t i = 0; i < numberFftBins; ++i) {
        out_smooth[i] += (out_log[i] - out_smooth[i]) * smoothness * dt;
    }

    return numberFftBins;
}
