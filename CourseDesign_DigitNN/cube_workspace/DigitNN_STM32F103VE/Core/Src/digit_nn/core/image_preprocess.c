/**
 * @file image_preprocess.c
 * @brief Touch-stroke filtering and 28x28 rasterization.
 * @author TODO
 * @date 2026-05-31
 */
#include "digit_nn/core/image_preprocess.h"

#include <stddef.h>
#include <string.h>

typedef struct {
    uint16_t min_x;
    uint16_t max_x;
    uint16_t min_y;
    uint16_t max_y;
} bounding_box_t;

static uint16_t absolute_diff_u16(uint16_t left, uint16_t right);
static status_code_t find_bounding_box(const stroke_buffer_t *buffer, bounding_box_t *box);
static void set_pixel_safe(digit_image_t *image, int16_t x, int16_t y, uint8_t value);
static void draw_line(digit_image_t *image, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
static void thicken_image(digit_image_t *image);
static void map_point_to_image(const touch_point_t *point, const bounding_box_t *box, int16_t *x, int16_t *y);

void preprocess_clear_strokes(stroke_buffer_t *buffer)
{
    if (buffer == NULL) {
        return;
    }

    buffer->count = 0U;
}

void preprocess_clear_image(digit_image_t *image)
{
    if (image == NULL) {
        return;
    }

    (void)memset(image->pixels, 0, sizeof(image->pixels));
}

status_code_t preprocess_add_point(stroke_buffer_t *buffer, const touch_point_t *point)
{
    touch_point_t *last_point;

    if ((buffer == NULL) || (point == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    if (buffer->count >= TOUCH_MAX_POINTS) {
        return STATUS_ERROR_BUFFER_FULL;
    }

    if (buffer->count > 0U) {
        last_point = &buffer->points[buffer->count - 1U];

        if ((last_point->pressed == point->pressed) &&
            (absolute_diff_u16(last_point->x, point->x) < TOUCH_MIN_MOVE_PIXELS) &&
            (absolute_diff_u16(last_point->y, point->y) < TOUCH_MIN_MOVE_PIXELS)) {
            return STATUS_SUCCESS;
        }
    }

    buffer->points[buffer->count] = *point;
    buffer->count++;

    return STATUS_SUCCESS;
}

status_code_t preprocess_render_strokes(const stroke_buffer_t *buffer, digit_image_t *image)
{
    bounding_box_t box;
    uint16_t index;
    int16_t last_x = 0;
    int16_t last_y = 0;
    uint8_t has_last = 0U;

    if ((buffer == NULL) || (image == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    preprocess_clear_image(image);

    if (find_bounding_box(buffer, &box) != STATUS_SUCCESS) {
        return STATUS_ERROR_EMPTY_INPUT;
    }

    for (index = 0U; index < buffer->count; index++) {
        const touch_point_t *point = &buffer->points[index];
        int16_t x;
        int16_t y;

        if (point->pressed == 0U) {
            has_last = 0U;
            continue;
        }

        map_point_to_image(point, &box, &x, &y);

        if (has_last != 0U) {
            draw_line(image, last_x, last_y, x, y);
        } else {
            set_pixel_safe(image, x, y, 255U);
        }

        last_x = x;
        last_y = y;
        has_last = 1U;
    }

    thicken_image(image);
    return STATUS_SUCCESS;
}

uint8_t preprocess_is_empty(const digit_image_t *image)
{
    uint16_t index;

    if (image == NULL) {
        return 1U;
    }

    for (index = 0U; index < DIGIT_IMAGE_SIZE; index++) {
        if (image->pixels[index] != 0U) {
            return 0U;
        }
    }

    return 1U;
}

static uint16_t absolute_diff_u16(uint16_t left, uint16_t right)
{
    return (left > right) ? (uint16_t)(left - right) : (uint16_t)(right - left);
}

static status_code_t find_bounding_box(const stroke_buffer_t *buffer, bounding_box_t *box)
{
    uint16_t index;
    uint8_t found = 0U;

    if ((buffer == NULL) || (box == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    box->min_x = 0xFFFFU;
    box->min_y = 0xFFFFU;
    box->max_x = 0U;
    box->max_y = 0U;

    for (index = 0U; index < buffer->count; index++) {
        const touch_point_t *point = &buffer->points[index];

        if (point->pressed == 0U) {
            continue;
        }

        if (point->x < box->min_x) {
            box->min_x = point->x;
        }

        if (point->x > box->max_x) {
            box->max_x = point->x;
        }

        if (point->y < box->min_y) {
            box->min_y = point->y;
        }

        if (point->y > box->max_y) {
            box->max_y = point->y;
        }

        found = 1U;
    }

    if (found == 0U) {
        return STATUS_ERROR_EMPTY_INPUT;
    }

    if ((box->max_x - box->min_x < PREPROCESS_MIN_BOX_SIZE) ||
        (box->max_y - box->min_y < PREPROCESS_MIN_BOX_SIZE)) {
        return STATUS_ERROR_EMPTY_INPUT;
    }

    return STATUS_SUCCESS;
}

static void set_pixel_safe(digit_image_t *image, int16_t x, int16_t y, uint8_t value)
{
    uint16_t index;

    if ((image == NULL) || (x < 0) || (y < 0) ||
        (x >= (int16_t)DIGIT_IMAGE_WIDTH) || (y >= (int16_t)DIGIT_IMAGE_HEIGHT)) {
        return;
    }

    index = (uint16_t)y * DIGIT_IMAGE_WIDTH + (uint16_t)x;

    if (value > image->pixels[index]) {
        image->pixels[index] = value;
    }
}

static void draw_line(digit_image_t *image, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t dx = (x0 < x1) ? (int16_t)(x1 - x0) : (int16_t)(x0 - x1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = (y0 < y1) ? (int16_t)(y0 - y1) : (int16_t)(y1 - y0);
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t error = (int16_t)(dx + dy);

    while (1) {
        int16_t error2;

        set_pixel_safe(image, x0, y0, 255U);

        if ((x0 == x1) && (y0 == y1)) {
            break;
        }

        error2 = (int16_t)(2 * error);

        if (error2 >= dy) {
            error = (int16_t)(error + dy);
            x0 = (int16_t)(x0 + sx);
        }

        if (error2 <= dx) {
            error = (int16_t)(error + dx);
            y0 = (int16_t)(y0 + sy);
        }
    }
}

static void thicken_image(digit_image_t *image)
{
    uint8_t copy[DIGIT_IMAGE_SIZE];
    uint16_t index;

    if (image == NULL) {
        return;
    }

    (void)memcpy(copy, image->pixels, sizeof(copy));

    for (index = 0U; index < DIGIT_IMAGE_SIZE; index++) {
        uint16_t x = index % DIGIT_IMAGE_WIDTH;
        uint16_t y = index / DIGIT_IMAGE_WIDTH;

        if (copy[index] == 0U) {
            continue;
        }

        set_pixel_safe(image, (int16_t)x - 1, (int16_t)y, 160U);
        set_pixel_safe(image, (int16_t)x + 1, (int16_t)y, 160U);
        set_pixel_safe(image, (int16_t)x, (int16_t)y - 1, 160U);
        set_pixel_safe(image, (int16_t)x, (int16_t)y + 1, 160U);
    }
}

static void map_point_to_image(const touch_point_t *point, const bounding_box_t *box, int16_t *x, int16_t *y)
{
    uint32_t width = (uint32_t)(box->max_x - box->min_x + 1U);
    uint32_t height = (uint32_t)(box->max_y - box->min_y + 1U);
    uint32_t max_side = (width > height) ? width : height;
    uint32_t scaled_x = ((uint32_t)(point->x - box->min_x) * PREPROCESS_TARGET_BOX_SIZE) / max_side;
    uint32_t scaled_y = ((uint32_t)(point->y - box->min_y) * PREPROCESS_TARGET_BOX_SIZE) / max_side;
    uint32_t offset_x = PREPROCESS_TARGET_PADDING;
    uint32_t offset_y = PREPROCESS_TARGET_PADDING;

    if (width < max_side) {
        offset_x += (PREPROCESS_TARGET_BOX_SIZE - ((width * PREPROCESS_TARGET_BOX_SIZE) / max_side)) / 2U;
    }

    if (height < max_side) {
        offset_y += (PREPROCESS_TARGET_BOX_SIZE - ((height * PREPROCESS_TARGET_BOX_SIZE) / max_side)) / 2U;
    }

    *x = (int16_t)(offset_x + scaled_x);
    *y = (int16_t)(offset_y + scaled_y);
}
