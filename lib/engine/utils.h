/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __UTILS_H__INCLUDED__
#define __UTILS_H__INCLUDED__

#include <vector>
#include <map>

#include "string.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
#define min(__a, __b) \
    ({ typeof(__a) a = __a; \
       typeof(__b) b = __b; \
       a < b ? a : b; })

#define foreach(i, c) for(typeof(c.begin()) i = c.begin(); i != c.end(); ++i)

/* */
void setLastErrorMessage(const String &ErrorMessage);

String getLastErrorMessage();

void clearLastErrorMessage();

void mdadmErrorLines(const String& output, std::vector<String>& lines);

/* */
int shell_cap(const String &s, String &r);

/**
 * @brief capture shell output as binary data
 *
 * @note	Function does not append trailing '\0'
 *
 * @return	`-1 for error or exit status of command
 */
int shell_cap(const String &s, void *buf, size_t &size);

/* */
int shell_output(const String &command, String &output, bool errorOutput = false);

/* */
int shell(const String &s);

/* */
int shell_command(const String &cmd);

/* */
int shellEx(const String &s, unsigned int linesNum = 1, unsigned int offset = 0);

/* */
int processExist(int pid, const String &s);

/* */
int readPidFile(const String &s, const String &proc);

/* */
inline unsigned char sg_make_byte(unsigned char *s) {
    return *s;
}

inline unsigned short sg_make_short(unsigned char *s) {
    unsigned short t = *s;
    return (t << 8) + *(s + 1);
}

inline unsigned int sg_make_int(unsigned char *s) {
    unsigned int t = sg_make_short(s);
    return (t << 16) + sg_make_short(s + 2);
}

void check_dots(String &s);

#endif /* __UTILS_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
