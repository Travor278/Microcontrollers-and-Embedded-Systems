/**
 * @file serial_protocol.c
 * @brief UART text protocol formatting and command handling.
 * @author TODO
 * @date 2026-05-31
 */
#include "serial_protocol.h"

#include <stdio.h>
#include <string.h>

static serial_write_callback_t serial_writer = NULL;

static const char *model_to_text(recognizer_model_t model);
static void write_line(const char *line);

void serial_protocol_set_writer(serial_write_callback_t callback)
{
    serial_writer = callback;
}

void serial_protocol_send_result(const recognizer_result_t *result)
{
    char line[96];

    if (result == NULL) {
        return;
    }

    (void)snprintf(line, sizeof(line), "RESULT,model=%s,label=%u,confidence=%u,time_us=%lu\r\n",
                   model_to_text(result->model),
                   (unsigned int)result->label,
                   (unsigned int)result->confidence_q100,
                   (unsigned long)result->elapsed_us);
    write_line(line);
}

void serial_protocol_send_test_summary(const test_summary_t *summary)
{
    char line[128];
    uint32_t accuracy_q100 = 0U;

    if (summary == NULL) {
        return;
    }

    if (summary->total_count > 0U) {
        accuracy_q100 = ((uint32_t)summary->correct_count * 10000UL) / summary->total_count;
    }

    (void)snprintf(line, sizeof(line),
                   "TEST,set=%s,model=%s,total=%u,correct=%u,accuracy=%lu,avg_time_us=%lu\r\n",
                   summary->set_name,
                   model_to_text(summary->model),
                   (unsigned int)summary->total_count,
                   (unsigned int)summary->correct_count,
                   (unsigned long)accuracy_q100,
                   (unsigned long)summary->average_time_us);
    write_line(line);
}

void serial_protocol_send_status(const char *state, const char *message)
{
    char line[128];

    if ((state == NULL) || (message == NULL)) {
        return;
    }

    (void)snprintf(line, sizeof(line), "STATUS,state=%s,message=%s\r\n", state, message);
    write_line(line);
}

status_code_t serial_protocol_handle_command(const char *line)
{
    if (line == NULL) {
        return STATUS_ERROR_PARAM;
    }

    if (strncmp(line, "CMD,CLEAR", 9U) == 0) {
        serial_protocol_send_status("idle", "clear_requested");
        return STATUS_SUCCESS;
    }

    if (strncmp(line, "CMD,MODEL,P", 11U) == 0) {
        recognizer_set_model(RECOGNIZER_MODEL_PERCEPTRON);
        serial_protocol_send_status("idle", "model_perceptron");
        return STATUS_SUCCESS;
    }

    if (strncmp(line, "CMD,MODEL,F", 11U) == 0) {
        recognizer_set_model(RECOGNIZER_MODEL_FNN);
        serial_protocol_send_status("idle", "model_fnn");
        return STATUS_SUCCESS;
    }

    if (strncmp(line, "CMD,MODEL,C", 11U) == 0) {
        recognizer_set_model(RECOGNIZER_MODEL_CNN);
        serial_protocol_send_status("idle", "model_cnn");
        return STATUS_SUCCESS;
    }

    if (strncmp(line, "CMD,INFO", 8U) == 0) {
        serial_protocol_send_status("idle", "digit_nn_ready");
        return STATUS_SUCCESS;
    }

    serial_protocol_send_status("idle", "unknown_command");
    return STATUS_ERROR_UNSUPPORTED;
}

static const char *model_to_text(recognizer_model_t model)
{
    if (model == RECOGNIZER_MODEL_CNN) {
        return "C";
    }

    return (model == RECOGNIZER_MODEL_FNN) ? "F" : "P";
}

static void write_line(const char *line)
{
    if ((serial_writer != NULL) && (line != NULL)) {
        serial_writer(line);
    }
}
