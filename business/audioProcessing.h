#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stddef.h>
#include <complex.h>
#include <raylib.h>
#include <fftw3.h>
#include "../infrastructure/config.h"

extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern float  out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];
extern fftwf_complex out_raw[FFT_SIZE];
extern float out_phase[FFT_SIZE];
extern float out_power[FFT_SIZE];

/*
void fft(float in[], size_t stride, float complex out[], size_t n);
float amp(float complex z);
void callback(void *bufferData, unsigned int frames);
size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[]);
*/

void initializeFFT();
void cleanupFFT();
void fft(float in[], size_t stride, float complex out[], size_t n);
float amp(float complex z);
float phase(fftwf_complex z);
float power(fftwf_complex z);
void callback(void *bufferData, unsigned int frames);
size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[], float out_phase[], float out_power[]);

#endif
