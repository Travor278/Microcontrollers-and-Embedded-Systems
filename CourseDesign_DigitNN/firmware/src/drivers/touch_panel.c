/**
 * @file touch_panel.c
 * @brief Touch panel adapter stub for the Wildfire STM32 board.
 * @author TODO
 * @date 2026-05-31
 */
#include "touch_panel.h"

#include <stddef.h>

status_code_t touch_panel_init(void)
{
    /*
     * Porting note:
     * Call XPT2046_Init() and XPT2046_Touch_Calibrate() from the Wildfire
     * touch driver here.
     */
    return STATUS_SUCCESS;
}

status_code_t touch_panel_read(touch_point_t *point)
{
    if (point == NULL) {
        return STATUS_ERROR_PARAM;
    }

    /*
     * Porting note:
     * Fill point->x, point->y, point->pressed, and point->tick_ms from
     * XPT2046_Get_TouchedPoint() plus the system tick.
     */
    point->pressed = 0U;
    return STATUS_ERROR_NOT_READY;
}
