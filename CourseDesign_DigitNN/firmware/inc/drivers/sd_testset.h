/**
 * @file sd_testset.h
 * @brief TF-card batch test interface.
 * @author TODO
 * @date 2026-05-31
 */
#ifndef SD_TESTSET_H
#define SD_TESTSET_H

#include "serial_protocol.h"
#include "status.h"

/**
 * @brief Run batch inference on one test-set directory.
 * @param[in] directory Directory path, such as "/mnist".
 * @param[out] summary Output summary.
 * @return Status code.
 */
status_code_t sd_testset_run(const char *directory, test_summary_t *summary);

#endif
