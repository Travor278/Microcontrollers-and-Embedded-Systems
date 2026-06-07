/**
 * @file recognizer.c
 * @brief Unified model selection and prediction wrapper.
 * @author TODO
 * @date 2026-05-31
 */
#include "recognizer.h"

#include <stddef.h>
#include "fnn.h"
#include "perceptron.h"

static recognizer_model_t active_model = RECOGNIZER_MODEL_PERCEPTRON;

void recognizer_set_model(recognizer_model_t model)
{
    if ((model == RECOGNIZER_MODEL_PERCEPTRON) || (model == RECOGNIZER_MODEL_FNN)) {
        active_model = model;
    }
}

recognizer_model_t recognizer_get_model(void)
{
    return active_model;
}

status_code_t recognizer_predict(const digit_image_t *image, recognizer_result_t *result)
{
    status_code_t status;

    if ((image == NULL) || (result == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    if (preprocess_is_empty(image) != 0U) {
        return STATUS_ERROR_EMPTY_INPUT;
    }

    result->model = active_model;
    result->elapsed_us = 0U;

    if (active_model == RECOGNIZER_MODEL_FNN) {
        fnn_result_t fnn_result;

        status = fnn_predict(image->pixels, &fnn_result);
        result->label = fnn_result.label;
        result->confidence_q100 = fnn_result.confidence_q100;
    } else {
        perceptron_result_t perceptron_result;

        status = perceptron_predict(image->pixels, &perceptron_result);
        result->label = perceptron_result.label;
        result->confidence_q100 = perceptron_result.confidence_q100;
    }

    return status;
}
