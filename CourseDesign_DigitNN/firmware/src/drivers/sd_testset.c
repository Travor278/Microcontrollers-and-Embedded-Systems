/**
 * @file sd_testset.c
 * @brief TF-card batch test adapter stub.
 * @author TODO
 * @date 2026-05-31
 */
#include "sd_testset.h"

#include <stddef.h>

status_code_t sd_testset_run(const char *directory, test_summary_t *summary)
{
    if ((directory == NULL) || (summary == NULL)) {
        return STATUS_ERROR_PARAM;
    }

    /*
     * Porting note:
     * Add FATFS and BMP reading here. For each image:
     * 1. Read grayscale pixels.
     * 2. Resize/normalize to digit_image_t when needed.
     * 3. Call recognizer_predict().
     * 4. Compare with label.txt.
     * 5. Accumulate accuracy and average time.
     */
    summary->set_name = directory;
    summary->model = recognizer_get_model();
    summary->total_count = 0U;
    summary->correct_count = 0U;
    summary->average_time_us = 0U;

    return STATUS_ERROR_NOT_READY;
}
