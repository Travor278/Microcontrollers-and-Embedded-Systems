/**
 * @file cnn.c
 * @brief Integer inference for the exported CNN or DS-CNN model.
 * @author TODO
 * @date 2026-06-07
 */
#include "cnn.h"

#include <stddef.h>
#include "CNN_Data.h"

#ifndef CNN_MODEL_KIND_STANDARD
#define CNN_MODEL_KIND_STANDARD 0U
#endif

#ifndef CNN_MODEL_KIND_DS_CNN
#define CNN_MODEL_KIND_DS_CNN 1U
#endif

#ifndef CNN_MODEL_KIND
#define CNN_MODEL_KIND CNN_MODEL_KIND_STANDARD
#endif

#if (CNN_MODEL_KIND == CNN_MODEL_KIND_DS_CNN)
static int32_t s_pool1[CNN_CONV1_OUT_CHANNELS][CNN_POOL1_HEIGHT][CNN_POOL1_WIDTH];
static int32_t s_ds1_depth[CNN_DS1_DW_CHANNELS][CNN_POOL1_HEIGHT][CNN_POOL1_WIDTH];
static int32_t s_pool2[CNN_DS1_PW_OUT_CHANNELS][CNN_POOL2_HEIGHT][CNN_POOL2_WIDTH];
static int32_t s_ds2_depth[CNN_DS2_DW_CHANNELS][CNN_POOL2_HEIGHT][CNN_POOL2_WIDTH];
static int32_t s_ds_features[CNN_DS2_PW_OUT_CHANNELS][CNN_POOL2_HEIGHT][CNN_POOL2_WIDTH];

static void run_conv1_pool(const uint8_t *pixels);
static void run_ds1_depthwise(void);
static void run_ds1_pointwise_pool(void);
static void run_ds2_depthwise(void);
static void run_ds2_pointwise(void);
static void run_ds_fc(cnn_result_t *result);
#else
static int32_t s_pool1[CNN_CONV1_OUT_CHANNELS][CNN_POOL1_HEIGHT][CNN_POOL1_WIDTH];
static int32_t s_pool2[CNN_CONV2_OUT_CHANNELS][CNN_POOL2_HEIGHT][CNN_POOL2_WIDTH];

static void run_conv1_pool(const uint8_t *pixels);
static void run_conv2_pool(void);
static void run_fc(cnn_result_t *result);
#endif

static void find_top2(const int32_t *logits, uint8_t *best_index, uint8_t *second_index);
static uint16_t estimate_confidence_q100(int32_t best_score, int32_t second_score);

status_code_t cnn_predict(const uint8_t *pixels, cnn_result_t *result)
{
    uint8_t best_index;
    uint8_t second_index;

    if ((pixels == NULL) || (result == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    run_conv1_pool(pixels);
#if (CNN_MODEL_KIND == CNN_MODEL_KIND_DS_CNN)
    run_ds1_depthwise();
    run_ds1_pointwise_pool();
    run_ds2_depthwise();
    run_ds2_pointwise();
    run_ds_fc(result);
#else
    run_conv2_pool();
    run_fc(result);
#endif

    find_top2(result->logits, &best_index, &second_index);
    result->label = best_index;
    result->confidence_q100 = estimate_confidence_q100(result->logits[best_index], result->logits[second_index]);

    return STATUS_SUCCESS;
}

static void run_conv1_pool(const uint8_t *pixels)
{
    uint8_t out_channel;
    uint8_t pool_y;
    uint8_t pool_x;
    uint8_t dy;
    uint8_t dx;
    uint8_t kernel_y;
    uint8_t kernel_x;

    for (out_channel = 0U; out_channel < CNN_CONV1_OUT_CHANNELS; out_channel++) {
        for (pool_y = 0U; pool_y < CNN_POOL1_HEIGHT; pool_y++) {
            for (pool_x = 0U; pool_x < CNN_POOL1_WIDTH; pool_x++) {
                int32_t max_value = 0;

                for (dy = 0U; dy < 2U; dy++) {
                    for (dx = 0U; dx < 2U; dx++) {
                        uint8_t y = (uint8_t)(pool_y * 2U + dy);
                        uint8_t x = (uint8_t)(pool_x * 2U + dx);
                        int32_t value = g_cnn_conv1_bias[out_channel];

                        for (kernel_y = 0U; kernel_y < CNN_KERNEL_SIZE; kernel_y++) {
                            int16_t source_y = (int16_t)y + (int16_t)kernel_y - 1;
                            if ((source_y < 0) || (source_y >= (int16_t)CNN_INPUT_HEIGHT)) {
                                continue;
                            }

                            for (kernel_x = 0U; kernel_x < CNN_KERNEL_SIZE; kernel_x++) {
                                int16_t source_x = (int16_t)x + (int16_t)kernel_x - 1;
                                if ((source_x < 0) || (source_x >= (int16_t)CNN_INPUT_WIDTH)) {
                                    continue;
                                }

                                value += (int32_t)pixels[(uint16_t)source_y * CNN_INPUT_WIDTH + (uint16_t)source_x] *
                                         (int32_t)g_cnn_conv1_weight[out_channel][kernel_y][kernel_x];
                            }
                        }

                        if (value > max_value) {
                            max_value = value;
                        }
                    }
                }

                s_pool1[out_channel][pool_y][pool_x] = max_value >> CNN_CONV1_SHIFT;
            }
        }
    }
}

#if (CNN_MODEL_KIND == CNN_MODEL_KIND_DS_CNN)
static void run_ds1_depthwise(void)
{
    uint8_t channel;
    uint8_t y;
    uint8_t x;
    uint8_t kernel_y;
    uint8_t kernel_x;

    for (channel = 0U; channel < CNN_DS1_DW_CHANNELS; channel++) {
        for (y = 0U; y < CNN_POOL1_HEIGHT; y++) {
            for (x = 0U; x < CNN_POOL1_WIDTH; x++) {
                int32_t value = g_cnn_ds1_depthwise_bias[channel];

                for (kernel_y = 0U; kernel_y < CNN_KERNEL_SIZE; kernel_y++) {
                    int16_t source_y = (int16_t)y + (int16_t)kernel_y - 1;
                    if ((source_y < 0) || (source_y >= (int16_t)CNN_POOL1_HEIGHT)) {
                        continue;
                    }

                    for (kernel_x = 0U; kernel_x < CNN_KERNEL_SIZE; kernel_x++) {
                        int16_t source_x = (int16_t)x + (int16_t)kernel_x - 1;
                        if ((source_x < 0) || (source_x >= (int16_t)CNN_POOL1_WIDTH)) {
                            continue;
                        }

                        value += s_pool1[channel][source_y][source_x] *
                                 (int32_t)g_cnn_ds1_depthwise_weight[channel][kernel_y][kernel_x];
                    }
                }

                if (value < 0) {
                    value = 0;
                }
                s_ds1_depth[channel][y][x] = value >> CNN_DS1_DW_SHIFT;
            }
        }
    }
}

static void run_ds1_pointwise_pool(void)
{
    uint8_t out_channel;
    uint8_t in_channel;
    uint8_t pool_y;
    uint8_t pool_x;
    uint8_t dy;
    uint8_t dx;

    for (out_channel = 0U; out_channel < CNN_DS1_PW_OUT_CHANNELS; out_channel++) {
        for (pool_y = 0U; pool_y < CNN_POOL2_HEIGHT; pool_y++) {
            for (pool_x = 0U; pool_x < CNN_POOL2_WIDTH; pool_x++) {
                int32_t max_value = 0;

                for (dy = 0U; dy < 2U; dy++) {
                    for (dx = 0U; dx < 2U; dx++) {
                        uint8_t y = (uint8_t)(pool_y * 2U + dy);
                        uint8_t x = (uint8_t)(pool_x * 2U + dx);
                        int32_t value = g_cnn_ds1_pointwise_bias[out_channel];

                        for (in_channel = 0U; in_channel < CNN_DS1_DW_CHANNELS; in_channel++) {
                            value += s_ds1_depth[in_channel][y][x] *
                                     (int32_t)g_cnn_ds1_pointwise_weight[out_channel][in_channel];
                        }

                        if (value > max_value) {
                            max_value = value;
                        }
                    }
                }

                s_pool2[out_channel][pool_y][pool_x] = max_value >> CNN_DS1_PW_SHIFT;
            }
        }
    }
}

static void run_ds2_depthwise(void)
{
    uint8_t channel;
    uint8_t y;
    uint8_t x;
    uint8_t kernel_y;
    uint8_t kernel_x;

    for (channel = 0U; channel < CNN_DS2_DW_CHANNELS; channel++) {
        for (y = 0U; y < CNN_POOL2_HEIGHT; y++) {
            for (x = 0U; x < CNN_POOL2_WIDTH; x++) {
                int32_t value = g_cnn_ds2_depthwise_bias[channel];

                for (kernel_y = 0U; kernel_y < CNN_KERNEL_SIZE; kernel_y++) {
                    int16_t source_y = (int16_t)y + (int16_t)kernel_y - 1;
                    if ((source_y < 0) || (source_y >= (int16_t)CNN_POOL2_HEIGHT)) {
                        continue;
                    }

                    for (kernel_x = 0U; kernel_x < CNN_KERNEL_SIZE; kernel_x++) {
                        int16_t source_x = (int16_t)x + (int16_t)kernel_x - 1;
                        if ((source_x < 0) || (source_x >= (int16_t)CNN_POOL2_WIDTH)) {
                            continue;
                        }

                        value += s_pool2[channel][source_y][source_x] *
                                 (int32_t)g_cnn_ds2_depthwise_weight[channel][kernel_y][kernel_x];
                    }
                }

                if (value < 0) {
                    value = 0;
                }
                s_ds2_depth[channel][y][x] = value >> CNN_DS2_DW_SHIFT;
            }
        }
    }
}

static void run_ds2_pointwise(void)
{
    uint8_t out_channel;
    uint8_t in_channel;
    uint8_t y;
    uint8_t x;

    for (out_channel = 0U; out_channel < CNN_DS2_PW_OUT_CHANNELS; out_channel++) {
        for (y = 0U; y < CNN_POOL2_HEIGHT; y++) {
            for (x = 0U; x < CNN_POOL2_WIDTH; x++) {
                int32_t value = g_cnn_ds2_pointwise_bias[out_channel];

                for (in_channel = 0U; in_channel < CNN_DS2_DW_CHANNELS; in_channel++) {
                    value += s_ds2_depth[in_channel][y][x] *
                             (int32_t)g_cnn_ds2_pointwise_weight[out_channel][in_channel];
                }

                if (value < 0) {
                    value = 0;
                }
                s_ds_features[out_channel][y][x] = value >> CNN_DS2_PW_SHIFT;
            }
        }
    }
}

static void run_ds_fc(cnn_result_t *result)
{
    uint8_t class_index;
    uint8_t channel;
    uint8_t y;
    uint8_t x;
    uint16_t feature_index;

    for (class_index = 0U; class_index < CNN_CLASS_COUNT; class_index++) {
        int32_t score = g_cnn_fc_bias[class_index];

        feature_index = 0U;
        for (channel = 0U; channel < CNN_DS2_PW_OUT_CHANNELS; channel++) {
            for (y = 0U; y < CNN_POOL2_HEIGHT; y++) {
                for (x = 0U; x < CNN_POOL2_WIDTH; x++) {
                    score += s_ds_features[channel][y][x] * (int32_t)g_cnn_fc_weight[class_index][feature_index];
                    feature_index++;
                }
            }
        }

        result->logits[class_index] = score;
    }
}
#else
static void run_conv2_pool(void)
{
    uint8_t out_channel;
    uint8_t in_channel;
    uint8_t pool_y;
    uint8_t pool_x;
    uint8_t dy;
    uint8_t dx;
    uint8_t kernel_y;
    uint8_t kernel_x;

    for (out_channel = 0U; out_channel < CNN_CONV2_OUT_CHANNELS; out_channel++) {
        for (pool_y = 0U; pool_y < CNN_POOL2_HEIGHT; pool_y++) {
            for (pool_x = 0U; pool_x < CNN_POOL2_WIDTH; pool_x++) {
                int32_t max_value = 0;

                for (dy = 0U; dy < 2U; dy++) {
                    for (dx = 0U; dx < 2U; dx++) {
                        uint8_t y = (uint8_t)(pool_y * 2U + dy);
                        uint8_t x = (uint8_t)(pool_x * 2U + dx);
                        int32_t value = g_cnn_conv2_bias[out_channel];

                        for (in_channel = 0U; in_channel < CNN_CONV2_IN_CHANNELS; in_channel++) {
                            for (kernel_y = 0U; kernel_y < CNN_KERNEL_SIZE; kernel_y++) {
                                int16_t source_y = (int16_t)y + (int16_t)kernel_y - 1;
                                if ((source_y < 0) || (source_y >= (int16_t)CNN_POOL1_HEIGHT)) {
                                    continue;
                                }

                                for (kernel_x = 0U; kernel_x < CNN_KERNEL_SIZE; kernel_x++) {
                                    int16_t source_x = (int16_t)x + (int16_t)kernel_x - 1;
                                    if ((source_x < 0) || (source_x >= (int16_t)CNN_POOL1_WIDTH)) {
                                        continue;
                                    }

                                    value += s_pool1[in_channel][source_y][source_x] *
                                             (int32_t)g_cnn_conv2_weight[out_channel][in_channel][kernel_y][kernel_x];
                                }
                            }
                        }

                        if (value > max_value) {
                            max_value = value;
                        }
                    }
                }

                s_pool2[out_channel][pool_y][pool_x] = max_value >> CNN_CONV2_SHIFT;
            }
        }
    }
}

static void run_fc(cnn_result_t *result)
{
    uint8_t class_index;
    uint8_t channel;
    uint8_t y;
    uint8_t x;
    uint16_t feature_index;

    for (class_index = 0U; class_index < CNN_CLASS_COUNT; class_index++) {
        int32_t score = g_cnn_fc_bias[class_index];

        feature_index = 0U;
        for (channel = 0U; channel < CNN_CONV2_OUT_CHANNELS; channel++) {
            for (y = 0U; y < CNN_POOL2_HEIGHT; y++) {
                for (x = 0U; x < CNN_POOL2_WIDTH; x++) {
                    score += s_pool2[channel][y][x] * (int32_t)g_cnn_fc_weight[class_index][feature_index];
                    feature_index++;
                }
            }
        }

        result->logits[class_index] = score;
    }
}
#endif

static void find_top2(const int32_t *logits, uint8_t *best_index, uint8_t *second_index)
{
    uint8_t index;

    *best_index = 0U;
    *second_index = 1U;

    if (logits[*second_index] > logits[*best_index]) {
        *best_index = 1U;
        *second_index = 0U;
    }

    for (index = 2U; index < CNN_CLASS_COUNT; index++) {
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
