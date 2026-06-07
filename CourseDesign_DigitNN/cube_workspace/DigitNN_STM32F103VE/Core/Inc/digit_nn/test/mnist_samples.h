/**
 * @file mnist_samples.h
 * @brief Embedded MNIST samples for offline model smoke tests.
 * @author generated
 */
#ifndef MNIST_SAMPLES_H
#define MNIST_SAMPLES_H

#include <stdint.h>

#define MNIST_SAMPLE_COUNT 8U
#define MNIST_SAMPLE_IMAGE_SIZE 784U

extern const uint8_t g_mnist_sample_images[MNIST_SAMPLE_COUNT][MNIST_SAMPLE_IMAGE_SIZE];
extern const uint8_t g_mnist_sample_labels[MNIST_SAMPLE_COUNT];

#endif
