/**
 * @file perceptron.h
 * @brief Integer inference for a single-layer perceptron MNIST model.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include <stdint.h>
#include "app_config.h"
#include "status.h"

typedef struct {
    int32_t logits[RECOGNIZER_CLASS_COUNT];
    uint8_t label;
    uint16_t confidence_q100;
} perceptron_result_t;

/**
 * @brief Run perceptron inference on a 28x28 grayscale image.
 * @param[in] pixels Image pixels, range 0..255.
 * @param[out] result Inference result.
 * @return Status code.
 */
status_code_t perceptron_predict(const uint8_t *pixels, perceptron_result_t *result);

#endif
