/**
 * @file recognizer.h
 * @brief Unified digit recognizer interface.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <stdint.h>
#include "app_config.h"
#include "image_preprocess.h"
#include "status.h"

typedef enum {
    RECOGNIZER_MODEL_PERCEPTRON = 0,
    RECOGNIZER_MODEL_FNN = 1,
    RECOGNIZER_MODEL_CNN = 2
} recognizer_model_t;

typedef struct {
    recognizer_model_t model;
    uint8_t label;
    uint16_t confidence_q100;
    uint32_t elapsed_us;
} recognizer_result_t;

/**
 * @brief Select the active model.
 * @param[in] model Model identifier.
 */
void recognizer_set_model(recognizer_model_t model);

/**
 * @brief Get the active model.
 * @return Active model identifier.
 */
recognizer_model_t recognizer_get_model(void);

/**
 * @brief Run the active model on a preprocessed image.
 * @param[in] image Input digit image.
 * @param[out] result Recognition result.
 * @return Status code.
 */
status_code_t recognizer_predict(const digit_image_t *image, recognizer_result_t *result);

#endif
