// audio_processing.h

#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stddef.h>
#include <complex.h>
#include <stdbool.h>

// Define FFT_SIZE as a power of 2
#define FFT_SIZE (1 << 14)

typedef struct {
    float in_raw[FFT_SIZE];
    float in_win[FFT_SIZE];
    float _Complex out_raw[FFT_SIZE];
    float out_log[FFT_SIZE];
    float out_smooth[FFT_SIZE];
    float out_phase[FFT_SIZE];
    float out_power[FFT_SIZE];
    size_t bufferIndex;
} AudioData;

typedef enum {
    TEST_SIGNAL_SINE,
    TEST_SIGNAL_MULTI_SINE,
    TEST_SIGNAL_CHIRP,
    TEST_SIGNAL_NOISE
} TestSignalType;

extern TestSignalType currentTestSignal;
extern bool testMode;

// Function declarations
void init_audio_data(AudioData *audioData);
void fft_iterative_with_twiddles(AudioData *audioData, size_t n);
void callback(void *bufferData, unsigned int frames);
void apply_window_function(const float input[], float output[], size_t n);
size_t ProcessFFT(AudioData *audioData);
void computePhase(AudioData *audioData, size_t n);
void computePowerSpectrum(AudioData *audioData, size_t n);
void detectPeaks(AudioData *audioData, size_t n, bool peaks[]);
void applyBandpassFilter(AudioData *audioData, size_t n, float lowCut, float highCut, float sampleRate);
void set_audio_data(AudioData *audioData);
float getPerceptualWeight(float frequency);
float getMaxPerceptualWeight(float minFreq, float maxFreq);

void generateWhiteNoise(float *buffer, size_t length);
void generateChirpSignal(float *buffer, size_t length, float startFreq, float endFreq, float sampleRate);
void generateMultiSineWave(float *buffer, size_t length, float *frequencies, size_t numFrequencies, float sampleRate);
void generateSineWave(float *buffer, size_t length, float frequency, float sampleRate);

#endif // AUDIO_PROCESSING_H
