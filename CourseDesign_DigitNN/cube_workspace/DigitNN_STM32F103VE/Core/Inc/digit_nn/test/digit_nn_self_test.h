/**
 * @file digit_nn_self_test.h
 * @brief Offline MNIST smoke test entry for the CubeIDE project.
 * @author TODO
 */
#ifndef DIGIT_NN_SELF_TEST_H
#define DIGIT_NN_SELF_TEST_H

#include <stdint.h>
#include "digit_nn/test/mnist_samples.h"

typedef struct {
    uint8_t expected_label;
    uint8_t perceptron_label;
    uint8_t fnn_label;
    uint16_t perceptron_confidence_q100;
    uint16_t fnn_confidence_q100;
} digit_nn_self_test_item_t;

extern volatile digit_nn_self_test_item_t g_digit_nn_self_test_results[MNIST_SAMPLE_COUNT];
extern volatile uint8_t g_digit_nn_self_test_perceptron_correct;
extern volatile uint8_t g_digit_nn_self_test_fnn_correct;

/**
 * @brief Run both models on embedded MNIST samples.
 */
void digit_nn_self_test_run(void);

#endif
