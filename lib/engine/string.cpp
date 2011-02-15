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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <cstdio>
#include <cstring>

#include "exception.h"
#include "string.h"

/* */
static int __utf8_sequence_type(unsigned char *buf)
{
    if (*buf < 0x7f) {
        return 1;
    }
    if ((*buf >> 5) == 0x06) {
        return 2;
    }
    if ((*buf >> 4) == 0x0e) {
        return 3;
    }
    if ((*buf >> 3) == 0x1e) {
        return 4;
    }
    return 0;
}

/* */
inline static unsigned char * __utf8_skip_byte(unsigned char *buf)
{
    if ((*buf >> 6) != 0x02) {
        throw E_INVALID_SEQUENCE;
    }
    return buf + 1;
}

/* */
static unsigned char * __utf8_skip_bytes(unsigned char *buf, int seqType)
{
    buf++;
    switch (seqType) {
    case 4:
        buf = __utf8_skip_byte(buf);
    case 3:
        buf = __utf8_skip_byte(buf);
    case 2:
        buf = __utf8_skip_byte(buf);
    case 1:
        break;
    default:
        throw E_INVALID_SEQUENCE;
    }
    return buf;
}

/* */
inline static unsigned char * __utf8_skip_sequence(unsigned char *buf)
{
    return __utf8_skip_bytes(buf, __utf8_sequence_type(buf));
}

/* */
static char * __utf8_offset(unsigned char *buf, unsigned int offset)
{
    try {
        while (*buf != '\0' && offset-- > 0) {
            buf = __utf8_skip_sequence(buf);
        }
    } catch (...) {
        // intentionally left blank
    }
    return reinterpret_cast<char *>(buf);
}

/* */
inline static char * __utf8_offset(char *buf, unsigned int offset)
{
    return __utf8_offset(reinterpret_cast<unsigned char *>(buf), offset);
}

/* */
inline static char * __utf8_offset(const char *buf, unsigned int offset)
{
    return __utf8_offset(const_cast<char *>(buf), offset);
}

/* */
static unsigned int __utf8_length(unsigned char *buf, unsigned int count)
{
    unsigned int result = 0, t;
    try {
        while (*buf != '\0' && count > 0) {
            buf = __utf8_skip_bytes(buf, t = __utf8_sequence_type(buf));
            if (t < count) {
                count = count - t;
            } else {
                count = 0;
            }
            result = result + 1;
        }
    } catch (...) {
    }
    return result;
}

/* */
inline static unsigned int __utf8_length(char *buf, unsigned int count = -1U)
{
    return __utf8_length(reinterpret_cast<unsigned char *>(buf), count);
}

/* */
void String::assign(const char *buf, unsigned int count)
{
    if (buf != 0) {
        if (count > 0) {
            count = __utf8_offset(const_cast<char *>(buf), count) - buf;
        }
    } else {
        count = 0;
    }
    __realloc(count + 1);
    if (buf) {
        __copy(buf, 0, count);
    } else {
        m_buffer[0] = '\0';
        m_size = m_length = 0;
    }
}

/* */
void String::assign(unsigned long long value)
{
    char t[42];
    snprintf(t, sizeof(t), "%llu", value);
    assign(t);
}

/* */
void String::assign(long long value)
{
    char t[42];
    snprintf(t, sizeof(t), "%lld", value);
    assign(t);
}

/* */
void String::append(const String &s, unsigned int count)
{
    if (this == &s) {
        return;
    }
    unsigned int size = s.size();
    if (size && count) {
        count = __utf8_offset(s.m_buffer, count) - s.m_buffer;
        if (size > count) {
            size = count;
        }
        __realloc(m_size + size + 1, true);
        __append(s, size);
    }
}

/* */
unsigned int String::find(const char *buf, unsigned int offset) const {
    char *p = __utf8_offset(m_buffer, offset);
    offset = p - m_buffer;
    if (buf != 0 && offset < m_size) {
        if ((p = __find(buf, offset)) != 0) {
            return __utf8_length(m_buffer, p - m_buffer);
        }
    }
    throw E_NOT_FOUND;
}

/* */
char * String::__offset(unsigned int offset) const
{
    return __utf8_offset(m_buffer, offset);
}

/* */
void String::__realloc(unsigned int buf_size, bool buf_copy)
{
    if (buf_size > m_capacity) {
        char *buf = new char[buf_size];
        if (buf_copy) {
            __duplicate(buf);
        }
        if (m_buffer) {
            delete [] m_buffer;
        }
        m_buffer = buf;
        m_capacity = buf_size;
    }
}

/* */
void String::__duplicate(char *buf) const {
    for (unsigned int i = 0; i < m_size; ++i) {
        buf[i] = m_buffer[i];
    }
    buf[m_size] = '\0';
}

/* */
void String::__copy(const char *buf, unsigned int offset, unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count && buf[i] != '\0'; ++i) {
        m_buffer[offset + i] = buf[i];
    }
    m_buffer[m_size = offset + i] = '\0';
    m_length = __utf8_length(m_buffer);
}

/* */
void String::__get(char *buf, unsigned int size, unsigned int offset) const
{
    offset = __utf8_offset(m_buffer, offset) - m_buffer;
    if (size < m_size - offset) {
        throw E_BUFFER_TOO_SMALL;
    }
    for (unsigned int i = 0; i < m_size - offset; ++i) {
        buf[i] = m_buffer[i + offset];
    }
    buf[m_size - offset] = '\0';
}

/* */
int String::__compare(const String &s, unsigned int offset) const
{
    if (m_length < s.length() - offset) {
        return -1;
    } else
    if (m_length > s.length() - offset) {
        return  1;
    } else {
        return strncmp(m_buffer, s.get(offset), m_size);
    }
}

/* */
char * String::__find(const char *buf, unsigned int offset) const
{
    unsigned int i, j = 0;
    i = __utf8_offset(buf, offset) - buf;
    while ((i + j) <= m_size) {
        if (buf[j] == '\0') {
            return m_buffer + i;
        }
        if (m_buffer[i + j] == buf[j]) {
            j++;
        } else {
            i++; j = 0;
        }
    }
    return 0;
}
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
