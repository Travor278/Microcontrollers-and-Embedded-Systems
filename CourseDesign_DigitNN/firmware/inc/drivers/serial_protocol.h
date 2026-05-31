/**
 * @file serial_protocol.h
 * @brief Text protocol for reporting recognition and test results over UART.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <stdint.h>
#include "recognizer.h"
#include "status.h"

typedef void (*serial_write_callback_t)(const char *text);

typedef struct {
    const char *set_name;
    recognizer_model_t model;
    uint16_t total_count;
    uint16_t correct_count;
    uint32_t average_time_us;
} test_summary_t;

/**
 * @brief Register the low-level UART text output function.
 * @param[in] callback Function used to transmit a null-terminated string.
 */
void serial_protocol_set_writer(serial_write_callback_t callback);

/**
 * @brief Send a recognition result frame.
 * @param[in] result Recognition result.
 */
void serial_protocol_send_result(const recognizer_result_t *result);

/**
 * @brief Send a batch test summary frame.
 * @param[in] summary Test summary.
 */
void serial_protocol_send_test_summary(const test_summary_t *summary);

/**
 * @brief Send a system status frame.
 * @param[in] state State name.
 * @param[in] message Short status message.
 */
void serial_protocol_send_status(const char *state, const char *message);

/**
 * @brief Parse and execute a received command line.
 * @param[in] line Null-terminated command line.
 * @return Status code.
 */
status_code_t serial_protocol_handle_command(const char *line);

#endif
