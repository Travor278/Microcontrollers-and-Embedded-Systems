/**
 * @file image_preprocess.h
 * @brief Convert touch strokes to a normalized 28x28 grayscale digit image.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef IMAGE_PREPROCESS_H
#define IMAGE_PREPROCESS_H

#include <stdint.h>
#include "app_config.h"
#include "status.h"

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t pressed;
    system_tick_t tick_ms;
} touch_point_t;

typedef struct {
    touch_point_t points[TOUCH_MAX_POINTS];
    uint16_t count;
} stroke_buffer_t;

typedef struct {
    uint8_t pixels[DIGIT_IMAGE_SIZE];
} digit_image_t;

/**
 * @brief Clear a stroke buffer.
 * @param[in,out] buffer Stroke buffer to clear.
 */
void preprocess_clear_strokes(stroke_buffer_t *buffer);

/**
 * @brief Clear a 28x28 grayscale image.
 * @param[in,out] image Image to clear.
 */
void preprocess_clear_image(digit_image_t *image);

/**
 * @brief Add a filtered touch point to the stroke buffer.
 * @param[in,out] buffer Destination stroke buffer.
 * @param[in] point New touch point.
 * @return Status code.
 */
status_code_t preprocess_add_point(stroke_buffer_t *buffer, const touch_point_t *point);

/**
 * @brief Build a normalized 28x28 digit image from touch strokes.
 * @param[in] buffer Source stroke buffer.
 * @param[out] image Output image.
 * @return Status code.
 */
status_code_t preprocess_render_strokes(const stroke_buffer_t *buffer, digit_image_t *image);

/**
 * @brief Check whether a digit image has no foreground pixels.
 * @param[in] image Image to inspect.
 * @return 1 when empty, otherwise 0.
 */
uint8_t preprocess_is_empty(const digit_image_t *image);

#endif
