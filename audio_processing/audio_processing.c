// audio_processing.c

#include "audio_processing.h"
#include "../presentation/playback.h"
#include <complex.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>

#define NUM_BINS 64
#define SAMPLE_RATE 44100.0f
#define EPSILON 1e-6f

// Precomputed window coefficients
static float window_coefficients[FFT_SIZE];
// Precomputed bit-reversal indices
static size_t bit_reversal_indices[FFT_SIZE];
// Precomputed twiddle factors
static float complex twiddle_factors[FFT_SIZE / 2];

TestSignalType currentTestSignal;
bool testMode = false;

/**
 * @brief Compute the window coefficients using a Hanning window function.
 *
 * This function precomputes the coefficients for a Hanning window,
 * which are used to reduce spectral leakage in the FFT.
*/
void compute_hann_window_coefficients(void) {
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i / (FFT_SIZE - 1);
        window_coefficients[i] = 0.5f - 0.5f * cosf(2.0f * M_PI * t);
    }
}

/**
 * @brief Compute the window coefficients using the Blackman-Harris window
 * function.
 *
 * This function precomputes the coefficients for a Blackman-Harris window,
 * which are used to reduce spectral leakage in the FFT.
 */
void compute_bh_window_coefficients(void) {
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i / (FFT_SIZE - 1);
        window_coefficients[i] = 0.35875f - 0.48829f * cosf(2.0f * M_PI * t) 
                                + 0.14128f * cosf(4.0f * M_PI * t) 
                                - 0.01168f * cosf(6.0f * M_PI * t);
    }
}

/**
 * @brief Compute bit-reversal indices for the FFT algorithm.
 *
 * @param n The size of the FFT (must be a power of 2).
 *
 * This function precomputes the bit-reversal indices required for rearranging
 * the input data before performing the FFT.
*/
void compute_bit_reversal_indices(size_t n) {
    size_t log2n = (size_t)log2(n);
    for (size_t i = 0; i < n; ++i) {
        size_t reversed = 0;
        for (size_t j = 0; j < log2n; ++j) {
            if (i & (1 << j)) {
                reversed |= 1 << (log2n - 1 - j);
            }
        }
        bit_reversal_indices[i] = reversed;
    }
}

/**
 * @brief Compute the twiddle fectors for the FFT algorithm.
 *
 * @param n The size of the FFT (must be a power of 2).
 *
 * This function precomputes the complex exponential (twiddle) factors
 * used in the FFT algorithm to improve performance.
*/
void compute_twiddle_factors(size_t n) {
    for (size_t k = 0; k < n / 2; ++k) {
        twiddle_factors[k] = cexpf(-2.0f * I * M_PI * k / n);
    }
}

/**
 * @brief Initialize the AudioData structure and precompute necessary
 * coefficients.
 *
 * @param audioData Pointer to the AudioData structure to initialize.
 *
 * This function initializes the audio data buffers and precomputes the window
 * coefficients, bit-reversal indices, and twiddle factors required for the
 * FFT.
 */
void init_audio_data(AudioData *audioData) {
    audioData->bufferIndex = 0;
    compute_bh_window_coefficients();
    compute_bit_reversal_indices(FFT_SIZE);
    compute_twiddle_factors(FFT_SIZE);

    memset(audioData->in_raw, 0, sizeof(audioData->in_raw));
    memset(audioData->in_win, 0, sizeof(audioData->in_win));
    memset(audioData->out_raw, 0, sizeof(audioData->out_raw));
    memset(audioData->out_log, 0, sizeof(audioData->out_log));
    memset(audioData->out_smooth, 0, sizeof(audioData->out_smooth));
    memset(audioData->out_phase, 0, sizeof(audioData->out_phase));
    memset(audioData->out_power, 0, sizeof(audioData->out_power));
}

/**
 * @brief Perform an in-place iterative FFT using precomputed twiddle factors
 * and bit-reversal indices.
 *
 * @param audioData Pointer to the AudioData structure containing input and
 * output buffers.
 * @param n The size of the FFT (must be a power of 2).
 *
 * This function performs an in-place Fast Fourier Transform (FFT) on the input
 * data, storing the complex frequency-domain results in the output buffer.
 */
void fft(AudioData *audioData, size_t n) {
    if (n == 0 || (n & (n - 1)) != 0) {
        // n must be a power of 2 and greater than 0
        fprintf(stderr, "Error: FFT size must be a power of 2 and greater than 0.\n");
        return;
    }

    // Copy input to output and apply bit-reversal permutation
    for (size_t i = 0; i < n; ++i) {
        audioData->out_raw[bit_reversal_indices[i]] = audioData->in_win[i];
    }

    // Iterative FFT computation
    size_t log2n = (size_t)log2(n);
    for (size_t s = 1; s <= log2n; ++s) {
        size_t m = 1 << s;
        size_t half_m = m / 2;
        size_t twiddle_step = n / m;
        for (size_t k = 0; k < n; k += m) {
            for (size_t j = 0; j < half_m; ++j) {
                float complex t = twiddle_factors[j * twiddle_step] * audioData->out_raw[k + j + half_m];
                float complex u = audioData->out_raw[k + j];
                audioData->out_raw[k + j] = u + t;
                audioData->out_raw[k + j + half_m] = u - t;
            }
        }
    }
}

static AudioData *audioDataPtr = NULL;

/**
 * @brief Set the global pointer to teh AudioData structure for use in callbacks.
 *
 * @param audioData Pointer to the AudioData structure to set.
 *
 * This function sets the global `audioDataPtr` to the provided AudioData
 * pointer, allowing teh callback function to access and modify audio data.
 */
void set_audio_data(AudioData *audioData) {
    audioDataPtr = audioData;
}

/**
 * @brief Audio processing callback function for handling incoming audio data.
 *
 * @param bufferData Pointer to the buffer containing audio frames.
 * @praram frames The number of frames in the buffer.
 *
 * This function is called whenever new audio data is available. It copies
 * the audio data into the `in_raw` buffer of the AudioData structure for
 * further processing
 */
void callback(void *bufferData, unsigned int frames) {
    if (audioDataPtr == NULL) return;

    if (frames > FFT_SIZE) frames = FFT_SIZE;
    //
    // Check if audio is playing or in test mode
    if (!isPlaying && !testMode) {
        // Do not write to the buffer
        return;
    }

    float (*inputBuffer)[2] = bufferData;

    for (size_t i = 0; i < frames; ++i) {
        audioDataPtr->in_raw[audioDataPtr->bufferIndex] = inputBuffer[i][0]; // Assuming mono input
        audioDataPtr->bufferIndex = (audioDataPtr->bufferIndex + 1) % FFT_SIZE;
    }
}

/**
 * @brief Apply a window function to the input signal
 *
 * @param input The input signal array
 * @param output The output array to store the windowed signal
 * @param The number of samples to process
 *
 * This function multiplies each sample of the input signal by the corresponding
 * window coefficient reducing spectral leakage in the FFT.
 */
void apply_window_function(const float input[], float output[], size_t n) {
    for (size_t i = 0; i < n; ++i) {
        output[i] = input[i] * window_coefficients[i];
    }
}

/**
 * @brief Process the FFT and compute the amplitude specturm for visualization
 * @param audioData Pointer to the AudioData structure containing audio buffers
 * @return The number of frequency bins computed
 *
 * This function handles the processing of audio data for visualization,
 * including generating test signals, applying window functios, performing the
 * FFT, computing logarithmically spaced frequency bins, and applying perceptual
 * weighting and smoothing.
 */

// Function to process FFT and compute amplitude spectrum
size_t ProcessFFT(AudioData *audioData) {
    float dt = GetFrameTime();

    float tempBuffer[FFT_SIZE];
    //
    // Check if audio is playing or in test mode
    if (!isPlaying && !testMode) {
        // No audio data to process; set output buffers to zero
        memset(audioData->out_smooth, 0, sizeof(audioData->out_smooth));
        return NUM_BINS;
    }

    if (testMode) {
        switch (currentTestSignal) {
        case TEST_SIGNAL_SINE:
            generateSineWave(tempBuffer, FFT_SIZE, 1000.0f, SAMPLE_RATE);
            break;
        case TEST_SIGNAL_MULTI_SINE: {
            float frequencies[] = {500.0f, 1500.0f};
            generateMultiSineWave(tempBuffer, FFT_SIZE, frequencies, 2, SAMPLE_RATE);
            break;
        }
        case TEST_SIGNAL_CHIRP:
            generateChirpSignal(tempBuffer, FFT_SIZE, 20.0f, 20000.0f, SAMPLE_RATE);
            break;
        case TEST_SIGNAL_NOISE:
            generateWhiteNoise(tempBuffer, FFT_SIZE);
            break;
        }
    } else {
        size_t index = audioData->bufferIndex;
        for (size_t i = 0; i < FFT_SIZE; ++i) {
            tempBuffer[i] = audioData->in_raw[(index + i) %FFT_SIZE];
        }
    }


    // Apply window function
    apply_window_function(tempBuffer, audioData->in_win, FFT_SIZE);

    // Perform FFT
    fft(audioData, FFT_SIZE);

    // Compute logarithmically spaced frequency bins
    size_t numberOfFftBins = NUM_BINS;
    float minFreq = 20.0f;    // Minimum frequency to visualize
    float maxFreq = 20000.0f; // Maximum frequency to visualize
    float logMinFreq = log10f(minFreq);
    float logMaxFreq = log10f(maxFreq);

    float maxAmplitude = 0.0f;
    float maxWeight = getMaxPerceptualWeight(minFreq, maxFreq);
    float weightScalingFactor = 0.5f;

    size_t fftSizeOver2 = FFT_SIZE / 2;

    for (size_t i = 0; i < numberOfFftBins; ++i) {
        float logFreqStart = logMinFreq + i * (logMaxFreq - logMinFreq) / numberOfFftBins;
        float logFreqEnd = logMinFreq + (i + 1) * (logMaxFreq - logMinFreq) / numberOfFftBins;

        float freqStart = powf(10.0f, logFreqStart);
        float freqEnd = powf(10.0f, logFreqEnd);
        float freqCenter = (freqStart + freqEnd) / 2.0f;

        size_t binStart = (size_t)((freqStart / (SAMPLE_RATE / 2.0f)) * fftSizeOver2);
        size_t binEnd = (size_t)((freqEnd / (SAMPLE_RATE / 2.0f)) * fftSizeOver2);
        if (binEnd > fftSizeOver2) binEnd = fftSizeOver2;
        if (binStart >= binEnd) binStart = (binEnd > 0) ? binEnd - 1 : 0;

        size_t binCount = binEnd - binStart;
        if (binCount == 0) binCount = 1; // Avoid division by zero

        float sum = 0.0f;
        for (size_t j = binStart; j < binEnd; ++j) {
            float amplitude = cabsf(audioData->out_raw[j]);
            sum += amplitude;
        }

        float binAmplitude = sum / binCount;

        float weight = getPerceptualWeight(freqCenter) / maxWeight;
        weight = powf(weight, weightScalingFactor);
        binAmplitude *= weight;

        audioData->out_log[i] = binAmplitude;

        if (binAmplitude > maxAmplitude) {
            maxAmplitude = binAmplitude;
        }
    }

    // Find the minimum and maximum log values
    float minLogAmplitude = INFINITY;
    float maxLogAmplitude = -INFINITY;
    for (size_t i = 0; i < numberOfFftBins; ++i) {
        audioData->out_log[i] = 20.0f * log10f(audioData->out_log[i] + EPSILON);
        if (audioData->out_log[i] < minLogAmplitude) minLogAmplitude = audioData->out_log[i];
        if (audioData->out_log[i] > maxLogAmplitude) maxLogAmplitude = audioData->out_log[i];
    }

    float floor = -60.0f;
    float range = maxLogAmplitude - floor;

    for (size_t i = 0; i < numberOfFftBins; ++i) {
        audioData->out_log[i] = (audioData->out_log[i] - floor) / range;
        audioData->out_log[i] = fmaxf(0.0f, fminf(1.0f, audioData->out_log[i]));
    }

    // Apply smoothing
    float smoothness = 10.0f;
    for (size_t i = 0; i < numberOfFftBins; ++i) {
        audioData->out_smooth[i] += (audioData->out_log[i] - audioData->out_smooth[i]) * smoothness * dt;
    }

    // Add code to print the amplitude spectrum
    printf("Amplitude Spectrum:\n");
    for (size_t i = 0; i < numberOfFftBins; ++i) {
        printf("%zu: %f\n", i, audioData->out_smooth[i]);
    }

    return numberOfFftBins;
}

/**
 * @brief Compute the phase spectrum from the FFT output.
 * @param audioData Pointer to the AudioData structure containing FFT results.
 * @param n The number of samples (FFT size).
 *
 * This function computes the phase angle (in radians) for each frequency bin
 * from the complex FFT output.
 */
void computePhase(AudioData *audioData, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        audioData->out_phase[i] = cargf(audioData->out_raw[i]);
    }
}

/**
 * @brief Compute the power spectrum from the FFT output.
 *
 * @param audioData Pointer to the AudioData structure containing FFT results.
 * @param n The number of samples (FFT size).
 *
 * This function computes the power (magnitude squared) of each frequency bin
 * from the complex FFT output.
 */
void computePowerSpectrum(AudioData *audioData, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        float amplitude = cabsf(audioData->out_raw[i]);
        audioData->out_power[i] = amplitude * amplitude;
    }
}

/**
 * @brief Detect peaks in the amplitude spectrum.
 *
 * @param audioData Pointer to the AudioData structure containing amplitude data.
 * @param n The number of samples (FFT size).
 * @param peak Array ot store peak detection results (true if peaks, false
 * otherwise).
 *
 * This function identifies local maxima in the amplitude spectrum,
 * marking positions where a frequency bin has a higher amplitude than its
 * immediate neighbors
 */
void detectPeaks(AudioData *audioData, size_t n, bool peaks[]) {
    peaks[0] = false;
    for (size_t i = 1; i < n - 1; ++i) {
        peaks[i] = (audioData->out_log[i] > audioData->out_log[i - 1]) &&
                   (audioData->out_log[i] > audioData->out_log[i + 1]);
    }
    peaks[n - 1] = false;
}

/**
 * @brief Apply a bandpass filter to the frequency-domain data.
 *
 * @param audioData Pointer to the AudioData structure containing FFT results.
 * @param n The number of samples (FFT size).
 * @param lowCut The lowercutoff frequency (Hz).
 * @param highCut The upper cutoff frequency (Hz).
 * @param sampleRate The sampling rate of the audio data (Hz).
 *
 * This function zeroes out frequency components outside the specifies frequency
 * range, effectively applying a bandpass filter in the frequency domain.
 */
void applyBandpassFilter(AudioData *audioData, size_t n, float lowCut, float highCut, float sampleRate) {
    for (size_t i = 0; i < n; ++i) {
        float frequency = (float)i / n * sampleRate;
        if (frequency < lowCut || frequency > highCut) {
            audioData->out_raw[i] = 0;
        }
    }
}

/**
 * @brief Calculate the perceptual wieghting (A-weighting for a gven frequency).
 *
 * @param frequency The frequency in Hz.
 * @return The perceptual weight for the frequency.
 *
 * This function computes a simplified A-weighting for a given frequency, which
 * models the human ear's sensitivity to different frequencies.
 */
float getPerceptualWeight(float frequency) {
    // Simplified A-weighting approximation
    float fSquared = frequency * frequency;
    float numerator = 12200.0f * 12200.0f * fSquared * fSquared;
    float denominator = (fSquared + 20.6f * 20.6f) * sqrtf((fSquared + 107.7f * 107.7f) * (fSquared + 737.9f * 737.9f)) * (fSquared + 12200.0f * 12200.0f);
    float aWeight = numerator / denominator;

    return aWeight;
}

/**
 * @brief Computer the maximum perceptual wight within a frequency range.
 *
 * @param minFreq The minimum frequency in Hz.
 * @param maxFreq The maximum frequency in Hz.
 * @return The maximum perceptual weight found within the frequency range.
 *
 * This function calculates the maximum A-weighting value over a specified
 * frequency range, which is used to normalize perceptual weights.
 */
float getMaxPerceptualWeight(float minFreq, float maxFreq) {
    float maxWeight = 0.0f;
    int numSamples = 1000; // Number of samples to compute within the range
    for (int i = 0; i <= numSamples; ++i) {
        float freq = minFreq + (maxFreq - minFreq) * i / numSamples;
        float weight = getPerceptualWeight(freq);
        if (weight > maxWeight) {
            maxWeight = weight;
        }
    }
    return maxWeight;
}

// test signal generation functions

void generateSineWave(float *buffer, size_t length, float frequency, float sampleRate) {
    float phase = 0.0f;
    float phaseIncrement = 2.0f * M_PI * frequency / sampleRate;

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = sinf(phase);
        phase += phaseIncrement;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    }
}

void generateMultiSineWave(float *buffer, size_t length, float *frequencies, size_t numFrequencies, float sampleRate) {
    static float phases[10] = {0}; // Adjust size as needed
    float phaseIncrements[10];

    // Calculate phase increments
    for (size_t j = 0; j < numFrequencies; ++j) {
        phaseIncrements[j] = 2.0f * M_PI * frequencies[j] / sampleRate;
    }

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = 0.0f;
        for (size_t j = 0; j < numFrequencies; ++j) {
            buffer[i] += sinf(phases[j]);
            phases[j] += phaseIncrements[j];
            if (phases[j] >= 2.0f * M_PI) phases[j] -= 2.0f * M_PI;
        }
        buffer[i] /= numFrequencies; // Normalize amplitude
    }
}

void generateChirpSignal(float *buffer, size_t length, float startFreq, float endFreq, float sampleRate) {
    float t, freq, phase = 0.0f;
    for (size_t i = 0; i < length; ++i) {
        t = (float)i / sampleRate;
        freq = startFreq + (endFreq - startFreq) * t / ((float)length / sampleRate);
        phase += 2.0f * M_PI * freq / sampleRate;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
        buffer[i] = sinf(phase);
    }
}

void generateWhiteNoise(float *buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; // Random value between -1 and 1
    }
}

#ifdef UNIT_TESTING
float* get_window_coefficients(void) {
    return window_coefficients;
}

size_t* get_bit_reversal_indices(void) {
    return bit_reversal_indices;
}

float complex* get_twiddle_factors(void) {
    return twiddle_factors;
}
#endif
