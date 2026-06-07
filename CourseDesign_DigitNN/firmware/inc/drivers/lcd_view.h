/**
 * @file lcd_view.h
 * @brief LCD drawing adapter for digit recognition UI.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef LCD_VIEW_H
#define LCD_VIEW_H

#include "image_preprocess.h"
#include "recognizer.h"
#include "status.h"

/**
 * @brief Initialize LCD and draw static UI.
 * @return Status code.
 */
status_code_t lcd_view_init(void);

/**
 * @brief Clear the handwriting canvas.
 */
void lcd_view_clear_canvas(void);

/**
 * @brief Draw a new stroke segment.
 * @param[in] previous Previous touch point.
 * @param[in] current Current touch point.
 */
void lcd_view_draw_stroke(const touch_point_t *previous, const touch_point_t *current);

/**
 * @brief Draw the normalized 28x28 preview.
 * @param[in] image Digit image.
 */
void lcd_view_draw_preview(const digit_image_t *image);

/**
 * @brief Draw recognition result.
 * @param[in] result Recognition result.
 */
void lcd_view_draw_result(const recognizer_result_t *result);

#endif
