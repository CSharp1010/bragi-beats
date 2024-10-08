// audio_processing.h

#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stddef.h>
#include <complex.h>
#include <stdbool.h>

// Define FFT_SIZE as a power of 2
#ifndef FFT_SIZE
#define FFT_SIZE (1 << 14) // Default to 16384
#endif

/**
 * @brief Structure to hold audio data for processing.
 */
typedef struct {
    float in_raw[FFT_SIZE];      /**< Raw input audio data */
    float in_win[FFT_SIZE];      /**< Windowed input audio data */
    float _Complex out_raw[FFT_SIZE]; /**< Raw FFT output (complex frequency domain data) */
    float out_log[FFT_SIZE];     /**< Logarithmically scaled amplitude spectrum */
    float out_smooth[FFT_SIZE];  /**< Smoothed amplitude spectrum for visualization */
    float out_phase[FFT_SIZE];   /**< Phase spectrum */
    float out_power[FFT_SIZE];   /**< Power spectrum */
    size_t bufferIndex;          /**< Current index in the circular buffer */
} AudioData;

/**
 * @brief Enumeration of test signal types for generating test audio data.
 */
typedef enum {
    TEST_SIGNAL_SINE,        /**< Single sine wave */
    TEST_SIGNAL_MULTI_SINE,  /**< Multiple sine waves summed together */
    TEST_SIGNAL_CHIRP,       /**< Chirp signal sweeping frequencies */
    TEST_SIGNAL_NOISE        /**< White noise */
} TestSignalType;

/**
 * @brief Enumeration of frequency scaling types for visualization.
 */
typedef enum {
    SCALE_LINEAR,       /**< Linear frequency scaling */
    SCALE_LOGARITHMIC,  /**< Logarithmic frequency scaling */
    SCALE_MEL           /**< Mel scale frequency scaling */
} FrequencyScale;

extern TestSignalType currentTestSignal; /**< Global variable to set the current test signal type */
extern bool testMode;                    /**< Global flag to indicate if test mode is active */

// Function declarations

/**
 * @brief Initialize the AudioData structure and precompute necessary coefficients.
 *
 * @param audioData Pointer to the AudioData structure to initialize.
 */
void init_audio_data(AudioData *audioData);

/**
 * @brief Audio processing callback function for handling incoming audio data.
 *
 * @param bufferData Pointer to the buffer containing audio frames.
 * @param frames The number of frames in the buffer.
 */
void callback(void *bufferData, unsigned int frames);

/**
 * @brief Apply a window function to the input signal.
 *
 * @param input The input signal array.
 * @param output The output array to store the windowed signal.
 * @param n The number of samples to process.
 */
void apply_window_function(const float input[], float output[], size_t n);

/**
 * @brief Process the FFT and compute the amplitude spectrum for visualization.
 *
 * @param audioData Pointer to the AudioData structure containing audio buffers.
 * @return The number of frequency bins computed.
 */
size_t ProcessFFT(AudioData *audioData);

/**
 * @brief Compute the phase spectrum from the FFT output.
 *
 * @param audioData Pointer to the AudioData structure containing FFT results.
 * @param n The number of samples (FFT size).
 */
void computePhase(AudioData *audioData, size_t n);

/**
 * @brief Compute the power spectrum from the FFT output.
 *
 * @param audioData Pointer to the AudioData structure containing FFT results.
 * @param n The number of samples (FFT size).
 */
void computePowerSpectrum(AudioData *audioData, size_t n);

/**
 * @brief Detect peaks in the amplitude spectrum.
 *
 * @param audioData Pointer to the AudioData structure containing amplitude data.
 * @param n The number of samples (FFT size).
 * @param peaks Array to store peak detection results (true if peak, false otherwise).
 */
void detectPeaks(AudioData *audioData, size_t n, bool peaks[]);

/**
 * @brief Apply a bandpass filter to the frequency-domain data.
 *
 * @param audioData Pointer to the AudioData structure containing FFT results.
 * @param n The number of samples (FFT size).
 * @param lowCut The lower cutoff frequency (Hz).
 * @param highCut The upper cutoff frequency (Hz).
 * @param sampleRate The sampling rate of the audio data (Hz).
 */
void applyBandpassFilter(AudioData *audioData, size_t n, float lowCut, float highCut, float sampleRate);

/**
 * @brief Set the global pointer to the AudioData structure for use in callbacks.
 *
 * @param audioData Pointer to the AudioData structure to set.
 */
void set_audio_data(AudioData *audioData);

/**
 * @brief Calculate the perceptual weighting (A-weighting) for a given frequency.
 *
 * @param frequency The frequency in Hz.
 * @return The perceptual weight for the frequency.
 */
float getPerceptualWeight(float frequency);

/**
 * @brief Compute the maximum perceptual weight within a frequency range.
 *
 * @param minFreq The minimum frequency in Hz.
 * @param maxFreq The maximum frequency in Hz.
 * @return The maximum perceptual weight found within the frequency range.
 */
float getMaxPerceptualWeight(float minFreq, float maxFreq);

/**
 * @brief Compute the window coefficients using a Hanning window function.
 */
void compute_hann_window_coefficients(void);

/**
 * @brief Compute the window coefficients using a Blackman-Harris  window function.
 */

void compute_bh_window_coefficients(void);


/**
 * @brief Compute bit-reversal indices for the FFT algorithm.
 *
 * @param n The size of the FFT (must be a power of 2).
 */
void compute_bit_reversal_indices(size_t n);

/**
 * @brief Compute the twiddle factors for the FFT algorithm.
 *
 * @param n The size of the FFT (must be a power of 2).
 */
void compute_twiddle_factors(size_t n);

/**
 * @brief Perform an in-place iterative FFT using precomputed twiddle factors and bit-reversal indices.
 *
 * @param audioData Pointer to the AudioData structure containing input and output buffers.
 * @param n The size of the FFT (must be a power of 2).
 */
void fft(AudioData *audioData, size_t n);

/**
 * @brief Generate white noise.
 *
 * @param buffer The output buffer to store the white noise samples.
 * @param length The number of samples to generate.
 */
void generateWhiteNoise(float *buffer, size_t length);

/**
 * @brief Generate a chirp signal that sweeps from startFreq to endFreq.
 *
 * @param buffer The output buffer to store the chirp signal samples.
 * @param length The number of samples to generate.
 * @param startFreq The starting frequency of the chirp (Hz).
 * @param endFreq The ending frequency of the chirp (Hz).
 * @param sampleRate The sampling rate (Hz).
 */
void generateChirpSignal(float *buffer, size_t length, float startFreq, float endFreq, float sampleRate);

/**
 * @brief Generate a multi-sine wave signal by summing multiple sine waves.
 *
 * @param buffer The output buffer to store the combined sine wave samples.
 * @param length The number of samples to generate.
 * @param frequencies Array of frequencies for the sine waves (Hz).
 * @param numFrequencies The number of frequencies in the array.
 * @param sampleRate The sampling rate (Hz).
 */
void generateMultiSineWave(float *buffer, size_t length, float *frequencies, size_t numFrequencies, float sampleRate);

/**
 * @brief Generate a sine wave signal.
 *
 * @param buffer The output buffer to store the sine wave samples.
 * @param length The number of samples to generate.
 * @param frequency The frequency of the sine wave (Hz).
 * @param sampleRate The sampling rate (Hz).
 */
void generateSineWave(float *buffer, size_t length, float frequency, float sampleRate);

#ifdef UNIT_TESTING
/**
 * @brief Get the precomputed window coefficients (for testing purposes).
 *
 * @return Pointer to the array of window coefficients.
 */
float* get_window_coefficients(void);

/**
 * @brief Get the precomputed bit-reversal indices (for testing purposes).
 *
 * @return Pointer to the array of bit-reversal indices.
 */
size_t* get_bit_reversal_indices(void);

/**
 * @brief Get the precomputed twiddle factors (for testing purposes).
 *
 * @return Pointer to the array of twiddle factors.
 */
float complex* get_twiddle_factors(void);
#endif

#endif // AUDIO_PROCESSING_H

