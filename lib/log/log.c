/*
 * IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING.
 * BY LOADING OR USING THIS SOFTWARE, YOU AGREE TO THE TERMS OF THIS INTEL
 * SOFTWARE LICENSE AGREEMENT.  IF YOU DO NOT WISH TO SO AGREE, DO NOT COPY,
 * INSTALL OR USE THIS SOFTWARE.
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 */

#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H == 1 */

#include <features.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "log.h"

/* initialize handle to trace file with value */
/* the same as error for standard library */
static FILE *g_log_stream = NULL;
static enum log_level g_log_level = LOG_NONE;

/* */
void log_init(enum log_level level, const char *path) {
    g_log_level = level;
    if (path) {
        if (g_log_stream) {
            fflush(g_log_stream);
            fclose(g_log_stream);
        }
        g_log_stream = fopen(path, "a+");
    }
}

/* */
void log_fini(void) {
    if (g_log_stream) {
        fflush(g_log_stream);
        fclose(g_log_stream);
    }
    g_log_level = LOG_NONE;
    g_log_stream = NULL;
}

/* */
void log_set_level(enum log_level level) {
    g_log_level = level;
}

/* */
enum log_level log_get_level(void) {
    return g_log_level;
}

/* */
void __log(enum log_level level, const char *format, ...) {
    struct tm tm;
    struct timeval tv;
    va_list vl;

    if ((g_log_level > level) || (format == NULL) || (g_log_stream == NULL))
        return;

    gettimeofday(&tv, 0);
    localtime_r(&tv.tv_sec, &tm);
    fprintf(g_log_stream, "%04d-%02d-%02d %02d:%02d:%02d-%04d [%d] ",
        1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
        tm.tm_sec, (int)tv.tv_usec, getpid());
    fflush(g_log_stream);
    va_start(vl, format);
    vfprintf(g_log_stream, format, vl);
    va_end(vl);
    fflush(g_log_stream);
    fprintf(g_log_stream, "\n");
    fflush(g_log_stream);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
