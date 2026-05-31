/**
 * @file fnn.h
 * @brief Integer inference for a fully connected neural network.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef FNN_H
#define FNN_H

#include <stdint.h>
#include "app_config.h"
#include "status.h"

typedef struct {
    int32_t logits[RECOGNIZER_CLASS_COUNT];
    uint8_t label;
    uint16_t confidence_q100;
} fnn_result_t;

/**
 * @brief Run FNN inference on a 28x28 grayscale image.
 * @param[in] pixels Image pixels, range 0..255.
 * @param[out] result Inference result.
 * @return Status code.
 */
status_code_t fnn_predict(const uint8_t *pixels, fnn_result_t *result);

#endif
