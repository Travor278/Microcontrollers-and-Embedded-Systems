/**
 * @file CNN_Data.h
 * @brief Cached quantized letter DS-CNN weights.
 */
#ifndef CNN_DATA_H
#define CNN_DATA_H

#include <stdint.h>

#define CNN_MODEL_KIND_STANDARD       0U
#define CNN_MODEL_KIND_DS_CNN         1U
#define CNN_MODEL_KIND                CNN_MODEL_KIND_DS_CNN
#define CNN_INPUT_WIDTH               28U
#define CNN_INPUT_HEIGHT              28U
#define CNN_KERNEL_SIZE               3U
#define CNN_POOL1_WIDTH               14U
#define CNN_POOL1_HEIGHT              14U
#define CNN_POOL2_WIDTH               7U
#define CNN_POOL2_HEIGHT              7U
#define CNN_CONV1_OUT_CHANNELS        12U
#define CNN_DS1_DW_CHANNELS           12U
#define CNN_DS1_PW_OUT_CHANNELS       24U
#define CNN_DS2_DW_CHANNELS           24U
#define CNN_DS2_PW_OUT_CHANNELS       32U
#define CNN_CONV2_IN_CHANNELS         CNN_DS1_PW_OUT_CHANNELS
#define CNN_CONV2_OUT_CHANNELS        CNN_DS2_PW_OUT_CHANNELS
#define CNN_FEATURE_SIZE              (CNN_DS2_PW_OUT_CHANNELS * CNN_POOL2_WIDTH * CNN_POOL2_HEIGHT)
#define CNN_CLASS_COUNT               26U
#define CNN_CONV1_SHIFT               8U
#define CNN_DS1_DW_SHIFT              8U
#define CNN_DS1_PW_SHIFT              8U
#define CNN_DS2_DW_SHIFT              8U
#define CNN_DS2_PW_SHIFT              8U
#define CNN_CONV2_SHIFT               CNN_DS2_PW_SHIFT

extern const int8_t g_cnn_conv1_weight[CNN_CONV1_OUT_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_conv1_bias[CNN_CONV1_OUT_CHANNELS];
extern const int8_t g_cnn_ds1_depthwise_weight[CNN_DS1_DW_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_ds1_depthwise_bias[CNN_DS1_DW_CHANNELS];
extern const int8_t g_cnn_ds1_pointwise_weight[CNN_DS1_PW_OUT_CHANNELS][CNN_DS1_DW_CHANNELS];
extern const int32_t g_cnn_ds1_pointwise_bias[CNN_DS1_PW_OUT_CHANNELS];
extern const int8_t g_cnn_ds2_depthwise_weight[CNN_DS2_DW_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_ds2_depthwise_bias[CNN_DS2_DW_CHANNELS];
extern const int8_t g_cnn_ds2_pointwise_weight[CNN_DS2_PW_OUT_CHANNELS][CNN_DS2_DW_CHANNELS];
extern const int32_t g_cnn_ds2_pointwise_bias[CNN_DS2_PW_OUT_CHANNELS];
extern const int8_t g_cnn_fc_weight[CNN_CLASS_COUNT][CNN_FEATURE_SIZE];
extern const int32_t g_cnn_fc_bias[CNN_CLASS_COUNT];

#endif
