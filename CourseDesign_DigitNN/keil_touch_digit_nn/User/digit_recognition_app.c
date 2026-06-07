#include "digit_recognition_app.h"

#include <stdio.h>
#include <string.h>

#include "palette.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "digit_nn/core/recognizer.h"

#define APP_STATUS_X          (PALETTE_START_X + 4U)
#define APP_STATUS_Y          2U
#define APP_STATUS_W          (LCD_X_LENGTH - PALETTE_START_X - 4U)
#define APP_STATUS_HEIGHT     34U

static stroke_buffer_t g_strokes;
static digit_image_t g_image;
static uint8_t g_has_active_stroke = 0U;

static void DrawStatusLine(const char *text);
static void DrawResult(uint8_t p_label, uint16_t p_conf, uint8_t f_label, uint16_t f_conf);

void DigitRecognition_Init(void)
{
    DigitRecognition_Clear();
}

void DigitRecognition_Clear(void)
{
    preprocess_clear_strokes(&g_strokes);
    preprocess_clear_image(&g_image);
    g_has_active_stroke = 0U;
    DrawStatusLine("Digit NN Ready");
}

void DigitRecognition_AddTouchPoint(int16_t x, int16_t y)
{
    touch_point_t point;

    if ((x <= (int16_t)PALETTE_START_X) || (y < 0) ||
        (x >= (int16_t)LCD_X_LENGTH) || (y >= (int16_t)LCD_Y_LENGTH)) {
        return;
    }

    point.x = (uint16_t)x;
    point.y = (uint16_t)y;
    point.pressed = 1U;
    point.tick_ms = 0U;
    (void)preprocess_add_point(&g_strokes, &point);
    g_has_active_stroke = 1U;
}

void DigitRecognition_EndStroke(void)
{
    touch_point_t point;

    if (g_has_active_stroke == 0U) {
        return;
    }

    point.x = 0U;
    point.y = 0U;
    point.pressed = 0U;
    point.tick_ms = 0U;
    (void)preprocess_add_point(&g_strokes, &point);
    g_has_active_stroke = 0U;
}

void DigitRecognition_Run(void)
{
    recognizer_result_t p_result;
    recognizer_result_t f_result;
    status_code_t status;

    DigitRecognition_EndStroke();

    status = preprocess_render_strokes(&g_strokes, &g_image);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("Input too small");
        printf("DigitNN: input too small, status=%d\r\n", (int)status);
        return;
    }

    recognizer_set_model(RECOGNIZER_MODEL_PERCEPTRON);
    status = recognizer_predict(&g_image, &p_result);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("Perceptron error");
        printf("DigitNN: perceptron error, status=%d\r\n", (int)status);
        return;
    }

    recognizer_set_model(RECOGNIZER_MODEL_FNN);
    status = recognizer_predict(&g_image, &f_result);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("FNN error");
        printf("DigitNN: fnn error, status=%d\r\n", (int)status);
        return;
    }

    DrawResult(p_result.label, p_result.confidence_q100,
               f_result.label, f_result.confidence_q100);

    printf("DigitNN result: Perceptron=%u conf=%u, FNN=%u conf=%u\r\n",
           (unsigned int)p_result.label,
           (unsigned int)p_result.confidence_q100,
           (unsigned int)f_result.label,
           (unsigned int)f_result.confidence_q100);
}

static void DrawStatusLine(const char *text)
{
    char line[32];

    (void)memset(line, 0, sizeof(line));
    (void)sprintf(line, "%s", text);

    LCD_SetFont(&Font8x16);
    LCD_SetColors(CL_BLACK, CL_WHITE);
    ILI9341_DrawRectangle(APP_STATUS_X, APP_STATUS_Y, APP_STATUS_W, APP_STATUS_HEIGHT, 1);
    ILI9341_DispString_EN(APP_STATUS_X + 2U, APP_STATUS_Y + 8U, line);
    LCD_SetColors(brush.color, CL_WHITE);
}

static void DrawResult(uint8_t p_label, uint16_t p_conf, uint8_t f_label, uint16_t f_conf)
{
    char line[32];

    (void)sprintf(line, "P:%u %u%%  F:%u %u%%",
                  (unsigned int)p_label,
                  (unsigned int)p_conf,
                  (unsigned int)f_label,
                  (unsigned int)f_conf);
    DrawStatusLine(line);
}
