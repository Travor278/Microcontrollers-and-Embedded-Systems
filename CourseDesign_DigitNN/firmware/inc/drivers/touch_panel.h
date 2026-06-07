/**
 * @file touch_panel.h
 * @brief Touch panel adapter interface.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef TOUCH_PANEL_H
#define TOUCH_PANEL_H

#include "image_preprocess.h"
#include "status.h"

/**
 * @brief Initialize the touch panel.
 * @return Status code.
 */
status_code_t touch_panel_init(void);

/**
 * @brief Read one touch sample.
 * @param[out] point Touch point sample.
 * @return Status code.
 */
status_code_t touch_panel_read(touch_point_t *point);

#endif
