#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stddef.h>
#include <complex.h>
#include <raylib.h>
#include "../infrastructure/config.h"

extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern float out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];
extern float _Complex out_raw[FFT_SIZE];
extern float out_phase[FFT_SIZE];
extern float out_power[FFT_SIZE];

void initializeFFT();
void cleanupFFT();
void fft(float in[], size_t stride, float _Complex out[], size_t n);
float amp(float _Complex z);
void callback(void *bufferData, unsigned int frames);
size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[]);

// New Functions for advanced audio processing
void computePhase(float _Complex out_raw[], float out_phase[], size_t n);
void computePowerSpectrum(float _Complex out_raw[], float out_power[], size_t n);
void detectPeaks(float out_log[], size_t n, bool peaks[]);
void applyBandpassFilter(float _Complex out_raw[], size_t n, float lowCut, float highCut);

#endif
