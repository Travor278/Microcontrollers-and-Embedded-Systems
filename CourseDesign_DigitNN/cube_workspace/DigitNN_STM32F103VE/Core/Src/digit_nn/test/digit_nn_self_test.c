/**
 * @file digit_nn_self_test.c
 * @brief Offline MNIST smoke test for Perceptron and FNN inference.
 * @author TODO
 */
#include "digit_nn/test/digit_nn_self_test.h"

#include <string.h>
#include "digit_nn/core/recognizer.h"

volatile digit_nn_self_test_item_t g_digit_nn_self_test_results[MNIST_SAMPLE_COUNT];
volatile uint8_t g_digit_nn_self_test_perceptron_correct = 0U;
volatile uint8_t g_digit_nn_self_test_fnn_correct = 0U;

void digit_nn_self_test_run(void)
{
    uint8_t index;
    digit_image_t image;
    recognizer_result_t result;

    g_digit_nn_self_test_perceptron_correct = 0U;
    g_digit_nn_self_test_fnn_correct = 0U;

    for (index = 0U; index < MNIST_SAMPLE_COUNT; index++) {
        (void)memcpy(image.pixels, g_mnist_sample_images[index], DIGIT_IMAGE_SIZE);
        g_digit_nn_self_test_results[index].expected_label = g_mnist_sample_labels[index];

        recognizer_set_model(RECOGNIZER_MODEL_PERCEPTRON);
        if (recognizer_predict(&image, &result) == STATUS_SUCCESS) {
            g_digit_nn_self_test_results[index].perceptron_label = result.label;
            g_digit_nn_self_test_results[index].perceptron_confidence_q100 = result.confidence_q100;
            if (result.label == g_mnist_sample_labels[index]) {
                g_digit_nn_self_test_perceptron_correct++;
            }
        }

        recognizer_set_model(RECOGNIZER_MODEL_FNN);
        if (recognizer_predict(&image, &result) == STATUS_SUCCESS) {
            g_digit_nn_self_test_results[index].fnn_label = result.label;
            g_digit_nn_self_test_results[index].fnn_confidence_q100 = result.confidence_q100;
            if (result.label == g_mnist_sample_labels[index]) {
                g_digit_nn_self_test_fnn_correct++;
            }
        }
    }
}
