/**
 * @file fnn.c
 * @brief Integer inference for the exported FNN model.
 * @author TODO
 * @date 2026-05-31
 */
#include "digit_nn/core/fnn.h"

#include <stddef.h>
#include "digit_nn/generated/FNN_Data.h"

static void find_top2(const int32_t *logits, uint8_t *best_index, uint8_t *second_index);
static uint16_t estimate_confidence_q100(int32_t best_score, int32_t second_score);

status_code_t fnn_predict(const uint8_t *pixels, fnn_result_t *result)
{
    static int32_t hidden[FNN_HIDDEN_SIZE];
    uint16_t hidden_index;
    uint16_t pixel_index;
    uint16_t class_index;
    uint8_t best_index;
    uint8_t second_index;

    if ((pixels == NULL) || (result == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    for (hidden_index = 0U; hidden_index < FNN_HIDDEN_SIZE; hidden_index++) {
        int32_t value = g_fnn_bias_1[hidden_index];

        for (pixel_index = 0U; pixel_index < FNN_INPUT_SIZE; pixel_index++) {
            value += (int32_t)g_fnn_weight_1[hidden_index][pixel_index] * (int32_t)pixels[pixel_index];
        }

        hidden[hidden_index] = (value > 0) ? value : 0;
    }

    for (class_index = 0U; class_index < FNN_CLASS_COUNT; class_index++) {
        int32_t score = g_fnn_bias_2[class_index];

        for (hidden_index = 0U; hidden_index < FNN_HIDDEN_SIZE; hidden_index++) {
            score += (int32_t)g_fnn_weight_2[class_index][hidden_index] * (hidden[hidden_index] >> FNN_HIDDEN_SHIFT);
        }

        result->logits[class_index] = score;
    }

    find_top2(result->logits, &best_index, &second_index);
    result->label = best_index;
    result->confidence_q100 = estimate_confidence_q100(result->logits[best_index], result->logits[second_index]);

    return STATUS_SUCCESS;
}

static void find_top2(const int32_t *logits, uint8_t *best_index, uint8_t *second_index)
{
    uint8_t index;

    *best_index = 0U;
    *second_index = 1U;

    if (logits[*second_index] > logits[*best_index]) {
        *best_index = 1U;
        *second_index = 0U;
    }

    for (index = 2U; index < RECOGNIZER_CLASS_COUNT; index++) {
        if (logits[index] > logits[*best_index]) {
            *second_index = *best_index;
            *best_index = index;
        } else if (logits[index] > logits[*second_index]) {
            *second_index = index;
        }
    }
}

static uint16_t estimate_confidence_q100(int32_t best_score, int32_t second_score)
{
    int32_t diff = best_score - second_score;
    int32_t magnitude = (best_score >= 0) ? best_score : -best_score;
    uint32_t confidence;

    if (diff <= 0) {
        return 0U;
    }

    if (magnitude < 1) {
        magnitude = 1;
    }

    confidence = (uint32_t)((diff * 100L) / (magnitude + diff));

    if (confidence > 100U) {
        confidence = 100U;
    }

    return (uint16_t)confidence;
}
