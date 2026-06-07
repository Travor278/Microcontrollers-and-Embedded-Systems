#ifndef DIGIT_RECOGNITION_APP_H
#define DIGIT_RECOGNITION_APP_H

#include "stm32f10x.h"

void DigitRecognition_Init(void);
void DigitRecognition_Clear(void);
void DigitRecognition_AddTouchPoint(int16_t x, int16_t y);
void DigitRecognition_EndStroke(void);
void DigitRecognition_Run(void);

#endif
