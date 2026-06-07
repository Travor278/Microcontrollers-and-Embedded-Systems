/**
 * @file status.h
 * @brief Common status codes for MCU modules.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_PARAM,
    STATUS_ERROR_EMPTY_INPUT,
    STATUS_ERROR_BUFFER_FULL,
    STATUS_ERROR_NOT_READY,
    STATUS_ERROR_IO,
    STATUS_ERROR_UNSUPPORTED
} status_code_t;

#endif
