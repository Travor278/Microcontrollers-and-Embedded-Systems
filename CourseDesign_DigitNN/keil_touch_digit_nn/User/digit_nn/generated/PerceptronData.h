/**
 * @file PerceptronData.h
 * @brief Quantized perceptron weights exported from tools/train_mnist.py.
 * @author generated
 */
#ifndef PERCEPTRON_DATA_H
#define PERCEPTRON_DATA_H

#include <stdint.h>

#define PERCEPTRON_INPUT_SIZE    784U
#define PERCEPTRON_CLASS_COUNT   10U

extern const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE];
extern const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT];

#endif
