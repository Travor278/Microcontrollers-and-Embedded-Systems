/**
 * @file cnn.h
 * @brief Integer inference for a tiny convolutional neural network.
 * @author TODO
 * @date 2026-06-07
 */
#ifndef CNN_H
#define CNN_H

#include <stdint.h>
#include "app_config.h"
#include "status.h"

typedef struct {
    int32_t logits[RECOGNIZER_CLASS_COUNT];
    uint8_t label;
    uint16_t confidence_q100;
} cnn_result_t;

/**
 * @brief Run Tiny-CNN inference on a 28x28 grayscale image.
 * @param[in] pixels Image pixels, range 0..255.
 * @param[out] result Inference result.
 * @return Status code.
 */
status_code_t cnn_predict(const uint8_t *pixels, cnn_result_t *result);

#endif
