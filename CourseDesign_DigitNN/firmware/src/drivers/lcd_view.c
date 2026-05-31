/**
 * @file lcd_view.c
 * @brief LCD drawing adapter stub.
 * @author TODO
 * @date 2026-05-31
 */
#include "lcd_view.h"

#include <stddef.h>

status_code_t lcd_view_init(void)
{
    /*
     * Porting note:
     * Call ILI9341_Init(), set scan direction, clear screen, and draw:
     * title, author/class, handwriting canvas, model status, result area.
     */
    return STATUS_SUCCESS;
}

void lcd_view_clear_canvas(void)
{
    /*
     * Porting note:
     * Clear LCD_CANVAS_LEFT/TOP/WIDTH/HEIGHT only, not the full screen.
     */
}

void lcd_view_draw_stroke(const touch_point_t *previous, const touch_point_t *current)
{
    (void)previous;
    (void)current;
    /*
     * Porting note:
     * Draw an LCD line from previous to current when both points are pressed.
     */
}

void lcd_view_draw_preview(const digit_image_t *image)
{
    (void)image;
    /*
     * Porting note:
     * Draw a scaled 28x28 preview beside the result area for debugging.
     */
}

void lcd_view_draw_result(const recognizer_result_t *result)
{
    (void)result;
    /*
     * Porting note:
     * Show model name, predicted label, confidence, and elapsed time.
     */
}
