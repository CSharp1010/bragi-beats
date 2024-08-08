#include "../business/audio_processing.h"
#include "../test/unity.h"

#define TEST_FFT_SIZE 16

static float test_input[TEST_FFT_SIZE] = {1, -1, 1, 0, 1, 0, -1, 0, 1, 0, -1, 1, 0, 1, 1, 0};
static float complex test_output[TEST_FFT_SIZE];

// Expected results from the FFT calculator
static float expected_real[TEST_FFT_SIZE] = {4.000000, 1.904548, -1.121320, -3.211111, 3.000000, -2.445744, 3.121320, 3.752307, 2.000000, 3.752307, 3.121320, -2.445744, 3.000000, -3.211111, -1.121320, 1.904548};
static float expected_imag[TEST_FFT_SIZE] = {0.000000, 1.230442, 0.707107, 1.158513, 1.000000, -0.841487, 0.707107, 3.230442, 0.000000, -3.230442, -0.707107, 0.841487, -1.000000, -1.158513, -0.707107, -1.230442};

void setUp(void) {}

void tearDown(void) {}

void test_fft(void) {
    fft(test_input, 1, test_output, TEST_FFT_SIZE);

    for (int i = 0; i < TEST_FFT_SIZE; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.001, expected_real[i], creal(test_output[i]));
        TEST_ASSERT_FLOAT_WITHIN(0.001, expected_imag[i], cimag(test_output[i]));
    }
}

void test_amp(void) {
    float complex z = 1.0 + 1.0 * I;
    float expected_magnitude = logf(1 + sqrtf(2));
    TEST_ASSERT_FLOAT_WITHIN(0.001, expected_magnitude, amp(z));
}

void test_amp_zero_input(void) {
    float complex z = 0.0 + 0.0 * I;
    float expected_magnitude = 0.0;
    TEST_ASSERT_FLOAT_WITHIN(0.001, expected_magnitude, amp(z));
}

void test_amp_real_input(void) {
    float complex z = 10.0 + 0.0 * I;
    float expected_magnitude = logf(1 + 10.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, expected_magnitude, amp(z));
}

void test_amp_imaginary_input(void) {
    float complex z = 0.0 + 5.0 * I;
    float expected_magnitude = logf(1 + 5.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, expected_magnitude, amp(z));
}

void test_amp_negative_parts(void) {
    float complex z = -3.0 - 4.0 * I;
    float expected_magnitude = logf(1 + 5.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, expected_magnitude, amp(z));
}

void test_amp_large_values(void) {
    float complex z = 1000.0 + 1000.0 * I;
    float expected_magnitude = logf(1 + sqrtf(1000000 + 1000000));
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected_magnitude, amp(z));
}

void test_amp_small_values(void) {
    float complex z = 0.001 + 0.001 * I;
    float expected_magnitude = logf(1 + sqrtf(0.001 * 0.001 + 0.001 * 0.001));
    TEST_ASSERT_FLOAT_WITHIN(0.0001, expected_magnitude, amp(z));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_fft);
    RUN_TEST(test_amp);
    RUN_TEST(test_amp_zero_input);
    RUN_TEST(test_amp_real_input);
    RUN_TEST(test_amp_imaginary_input);
    RUN_TEST(test_amp_negative_parts);
    RUN_TEST(test_amp_large_values);
    RUN_TEST(test_amp_small_values);

    return UNITY_END();
}
