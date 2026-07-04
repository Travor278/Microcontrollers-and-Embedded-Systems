#include "digit_recognition_app.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "palette.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "digit_nn/core/recognizer.h"

#define APP_STATUS_X          (PALETTE_START_X + 4U)
#define APP_STATUS_Y          2U
#define APP_STATUS_W          (LCD_X_LENGTH - PALETTE_START_X - 4U)
#define APP_STATUS_HEIGHT     34U
#define APP_SERIAL_BOOT_INFO_ENABLE    1U
#define APP_SERIAL_TOUCH_STREAM_ENABLE 1U
#define APP_SERIAL_REC_FRAME_ENABLE    1U
#define APP_SERIAL_POINT_STEP 2U

static stroke_buffer_t g_strokes;
static digit_image_t g_image;
static uint8_t g_has_active_stroke = 0U;
static uint8_t g_serial_point_count = 0U;
static uint8_t g_serial_has_last = 0U;
static uint16_t g_serial_last_x = 0U;
static uint16_t g_serial_last_y = 0U;

static void DrawStatusLine(const char *text);
static void DrawResult(const recognizer_result_t *p_result,
                       const recognizer_result_t *f_result,
                       const recognizer_result_t *c_result);
#if APP_SERIAL_TOUCH_STREAM_ENABLE
static void EmitTouchPoint(uint16_t x, uint16_t y);
#endif
#if APP_SERIAL_REC_FRAME_ENABLE
static void EmitImageFrame(const digit_image_t *image);
static void SerialPutHexByte(uint8_t value);
#endif

void DigitRecognition_Init(void)
{
    DigitRecognition_Clear();
}

void DigitRecognition_Clear(void)
{
    preprocess_clear_strokes(&g_strokes);
    preprocess_clear_image(&g_image);
    g_has_active_stroke = 0U;
    g_serial_point_count = 0U;
    g_serial_has_last = 0U;
    DrawStatusLine("Ready: draw digit");
#if APP_SERIAL_BOOT_INFO_ENABLE
    printf("INFO,fw=DigitNN_Touch,proto=touch_stream_v1,image=%ux%u\r\n",
           (unsigned int)DIGIT_IMAGE_WIDTH,
           (unsigned int)DIGIT_IMAGE_HEIGHT);
    printf("CLEAR\r\n");
    printf("STATUS,state=idle,message=ready,proto=touch_stream_v1\r\n");
#endif
}

void DigitRecognition_AddTouchPoint(int16_t x, int16_t y)
{
    touch_point_t point;
    status_code_t status;

    if ((x <= (int16_t)PALETTE_START_X) || (y < 0) ||
        (x >= (int16_t)LCD_X_LENGTH) || (y >= (int16_t)LCD_Y_LENGTH)) {
        return;
    }

    point.x = (uint16_t)x;
    point.y = (uint16_t)y;
    point.pressed = 1U;
    point.tick_ms = 0U;
    status = preprocess_add_point(&g_strokes, &point);
    if (status == STATUS_SUCCESS) {
        g_has_active_stroke = 1U;
#if APP_SERIAL_TOUCH_STREAM_ENABLE
        EmitTouchPoint(point.x, point.y);
#endif
    }
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
    g_serial_point_count = 0U;
    g_serial_has_last = 0U;
#if APP_SERIAL_TOUCH_STREAM_ENABLE
    printf("STROKE,end=1\r\n");
#endif
}

void DigitRecognition_Run(void)
{
    recognizer_result_t p_result;
    recognizer_result_t f_result;
    recognizer_result_t c_result;
    status_code_t status;

    DigitRecognition_EndStroke();

    status = preprocess_render_strokes(&g_strokes, &g_image);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("Input too small");
#if APP_SERIAL_REC_FRAME_ENABLE
        printf("STATUS,state=idle,message=input_too_small,status=%d\r\n", (int)status);
#endif
        printf("DigitNN: input too small, status=%d\r\n", (int)status);
        return;
    }

#if APP_SERIAL_REC_FRAME_ENABLE
    EmitImageFrame(&g_image);
#endif

    recognizer_set_model(RECOGNIZER_MODEL_PERCEPTRON);
    status = recognizer_predict(&g_image, &p_result);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("Perceptron error");
#if APP_SERIAL_REC_FRAME_ENABLE
        printf("STATUS,state=idle,message=perceptron_error,status=%d\r\n", (int)status);
#endif
        printf("DigitNN: perceptron error, status=%d\r\n", (int)status);
        return;
    }

    recognizer_set_model(RECOGNIZER_MODEL_FNN);
    status = recognizer_predict(&g_image, &f_result);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("FNN error");
#if APP_SERIAL_REC_FRAME_ENABLE
        printf("STATUS,state=idle,message=fnn_error,status=%d\r\n", (int)status);
#endif
        printf("DigitNN: fnn error, status=%d\r\n", (int)status);
        return;
    }

    recognizer_set_model(RECOGNIZER_MODEL_CNN);
    status = recognizer_predict(&g_image, &c_result);
    if (status != STATUS_SUCCESS) {
        DrawStatusLine("CNN error");
#if APP_SERIAL_REC_FRAME_ENABLE
        printf("STATUS,state=idle,message=cnn_error,status=%d\r\n", (int)status);
#endif
        printf("DigitNN: cnn error, status=%d\r\n", (int)status);
        return;
    }

    DrawResult(&p_result, &f_result, &c_result);

#if APP_SERIAL_REC_FRAME_ENABLE
    printf("RESULT,model=P,label=%u,confidence=%u,time_us=0\r\n",
           (unsigned int)p_result.label,
           (unsigned int)p_result.confidence_q100);
    printf("RESULT,model=F,label=%u,confidence=%u,time_us=0\r\n",
           (unsigned int)f_result.label,
           (unsigned int)f_result.confidence_q100);
    printf("RESULT,model=C,label=%u,confidence=%u,time_us=0\r\n",
           (unsigned int)c_result.label,
           (unsigned int)c_result.confidence_q100);

    printf("DigitNN result: Perceptron=%u conf=%u, FNN=%u conf=%u, CNN=%u conf=%u\r\n",
           (unsigned int)p_result.label,
           (unsigned int)p_result.confidence_q100,
           (unsigned int)f_result.label,
           (unsigned int)f_result.confidence_q100,
           (unsigned int)c_result.label,
           (unsigned int)c_result.confidence_q100);
#endif
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

static void DrawResult(const recognizer_result_t *p_result,
                       const recognizer_result_t *f_result,
                       const recognizer_result_t *c_result)
{
    char line[32];

    (void)sprintf(line, "P:%u %u%% F:%u %u%% C:%u %u%%",
                  (unsigned int)p_result->label,
                  (unsigned int)p_result->confidence_q100,
                  (unsigned int)f_result->label,
                  (unsigned int)f_result->confidence_q100,
                  (unsigned int)c_result->label,
                  (unsigned int)c_result->confidence_q100);
    DrawStatusLine(line);
}

#if APP_SERIAL_TOUCH_STREAM_ENABLE
static void EmitTouchPoint(uint16_t x, uint16_t y)
{
    if ((g_serial_has_last != 0U) &&
        (x == g_serial_last_x) &&
        (y == g_serial_last_y)) {
        return;
    }

    g_serial_point_count++;
    if ((g_serial_has_last != 0U) &&
        (g_serial_point_count < APP_SERIAL_POINT_STEP)) {
        return;
    }

    g_serial_point_count = 0U;
    g_serial_has_last = 1U;
    g_serial_last_x = x;
    g_serial_last_y = y;

    printf("POINT,x=%u,y=%u\r\n",
           (unsigned int)x,
           (unsigned int)y);
}
#endif

#if APP_SERIAL_REC_FRAME_ENABLE
static void EmitImageFrame(const digit_image_t *image)
{
    uint16_t index;

    if (image == NULL) {
        return;
    }

    printf("IMAGE,w=%u,h=%u,data=",
           (unsigned int)DIGIT_IMAGE_WIDTH,
           (unsigned int)DIGIT_IMAGE_HEIGHT);

    for (index = 0U; index < DIGIT_IMAGE_SIZE; index++) {
        SerialPutHexByte(image->pixels[index]);
    }

    printf("\r\n");
}

static void SerialPutHexByte(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    (void)putchar(hex[(value >> 4U) & 0x0FU]);
    (void)putchar(hex[value & 0x0FU]);
}
#endif
