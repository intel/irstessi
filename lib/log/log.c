
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
void __log(enum log_level level, const char *message) {
    struct tm tm;
    struct timeval tv;

    if ((g_log_level < level) || (message == NULL) || (g_log_stream == NULL))
        return;

    gettimeofday(&tv, 0);
    localtime_r(&tv.tv_sec, &tm);
    fprintf(g_log_stream, "%04d-%02d-%02d %02d:%02d:%02d-%04d [%d] ",
        1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
        tm.tm_sec, (int)tv.tv_usec, getpid());
    fflush(g_log_stream);
    fputs(message, g_log_stream);
    fflush(g_log_stream);
    fprintf(g_log_stream, "\n");
    fflush(g_log_stream);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
