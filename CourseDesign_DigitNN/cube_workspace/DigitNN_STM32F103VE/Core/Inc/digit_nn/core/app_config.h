/**
 * @file app_config.h
 * @brief Shared constants for the handwritten digit recognition firmware.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

#define DIGIT_IMAGE_WIDTH              28U
#define DIGIT_IMAGE_HEIGHT             28U
#define DIGIT_IMAGE_SIZE               (DIGIT_IMAGE_WIDTH * DIGIT_IMAGE_HEIGHT)

#define TOUCH_MAX_POINTS               256U
#define TOUCH_MIN_MOVE_PIXELS          2U
#define TOUCH_PEN_UP_TIMEOUT_MS        450U

#define LCD_CANVAS_LEFT                12U
#define LCD_CANVAS_TOP                 42U
#define LCD_CANVAS_WIDTH               216U
#define LCD_CANVAS_HEIGHT              216U

#define PREPROCESS_TARGET_BOX_SIZE     20U
#define PREPROCESS_TARGET_PADDING      4U
#define PREPROCESS_MIN_BOX_SIZE        4U

#define RECOGNIZER_CLASS_COUNT         10U

typedef uint32_t system_tick_t;

#endif
