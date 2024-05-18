#include "audioProcessing.h"
#include "../infrastructure/globals.h"

#include <complex.h>
#include <math.h>
#include <string.h>
#include <assert.h>

float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
float complex out_raw[FFT_SIZE];
float  out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];

void fft(float in[], size_t stride, float complex out[], size_t n) {
    assert(n > 0);

    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft(in, stride*2, out, n/2);
    fft(in + stride, stride*2, out + n/2, n/2);

    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k / n;

        float complex v = cexp(-2 * I * PI * t) * out[k + n / 2];

        float complex e = out[k];

        out[k] = e + v;

        out[k + n / 2] = e - v;
    }
}

float amp(float complex z) {
    float magnitude = cabs(z);
    return logf(1 + magnitude);
}

void callback(void *bufferData, unsigned int frames) {
    if (frames > FFT_SIZE) frames = FFT_SIZE;

    float (*fs)[2] = bufferData;

    for (size_t i = 0; i < frames; ++i) {
        memmove(in_raw, in_raw + 1, (FFT_SIZE-1)*sizeof(in_raw[0]));
        in_raw[FFT_SIZE-1] = fs[i][0];
    }
}

size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[]) {

    float dt = GetFrameTime();

    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i/(FFT_SIZE-1);
        float hann = 0.5 - 0.5*cosf(2*PI*t);
        in_win[i] = in_raw[i]*hann;
    }

    fft(in_win, 1, out_raw, FFT_SIZE);

    float step = 1.08;
    float lowf = 1.0f;
    size_t numberFftBins = 0;
    float max_amp = 1.0f;

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
