/**
 * @file FNN_Data.h
 * @brief Quantized FNN weights exported from tools/train_mnist.py.
 * @author generated
 */
#ifndef FNN_DATA_H
#define FNN_DATA_H

#include <stdint.h>

#define FNN_INPUT_SIZE     784U
#define FNN_HIDDEN_SIZE    64U
#define FNN_CLASS_COUNT    10U
#define FNN_HIDDEN_SHIFT   8U

extern const int8_t g_fnn_weight_1[FNN_HIDDEN_SIZE][FNN_INPUT_SIZE];
extern const int32_t g_fnn_bias_1[FNN_HIDDEN_SIZE];
extern const int8_t g_fnn_weight_2[FNN_CLASS_COUNT][FNN_HIDDEN_SIZE];
extern const int32_t g_fnn_bias_2[FNN_CLASS_COUNT];

#endif
