#include "../business/audioProcessing.h"
#include "../test/unity.h"

#define TEST_FFT_SIZE 15

static float test_input[TEST_FFT_SIZE] = {1, 1, -1, 1, 0, 1, 0, -1, 0, 1, 0, -1, 1, 0, 1};
static float complex test_output[TEST_FFT_SIZE];

void setUp(void) {

}

void tearDown(void) {

}

void test_fft(void) {
    fft(test_input, 1,  test_output, TEST_FFT_SIZE);

    TEST_ASSERT_EQUAL_FLOAT(0, creal(test_output[0]));
    TEST_ASSERT_EQUAL_FLOAT(-TEST_FFT_SIZE, cimag(test_output[0]));
}

void test_amp(void) {
    float complex z = 1.0 + 1.0 * I;
    float expected_magnitude = logf(1 + sqrtf(2));
    TEST_ASSERT_FLOAT_WITHIN(0.001, expected_magnitude, amp(z));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_fft);
    RUN_TEST(test_amp);

    return UNITY_END();
}
