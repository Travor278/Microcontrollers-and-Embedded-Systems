/**
 * @file RecognitionDomain.h
 * @brief Active recognition domain for the shared STM32 firmware shell.
 */
#ifndef RECOGNITION_DOMAIN_H
#define RECOGNITION_DOMAIN_H

#define RECOGNITION_DOMAIN_DIGIT   1U
#define RECOGNITION_DOMAIN_LETTER  2U

#define RECOGNITION_DOMAIN         RECOGNITION_DOMAIN_LETTER
#define RECOGNIZER_CLASS_COUNT     26U
#define RECOGNIZER_LABEL_BASE      'A'
#define RECOGNIZER_DOMAIN_NAME     "LetterNN"
#define RECOGNIZER_READY_TEXT      "Ready: draw letter"

#endif
