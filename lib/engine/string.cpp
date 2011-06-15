
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>

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
inline static unsigned char * __utf8_skip_invalid_sequence(unsigned char *buf)
{
    while (*buf != '\0') {
        if (__utf8_sequence_type(buf) != 0) {
            break;
        }
        ++buf;
    }
    return buf;
}

/* */
inline static unsigned char * __utf8_skip_sequence(unsigned char *buf)
{
    return __utf8_skip_bytes(buf, __utf8_sequence_type(buf));
}

/* */
static char * __utf8_offset(unsigned char *buf, unsigned int offset = -1U)
{
    try {
        while (*buf != '\0') {
            if (offset == 0) {
                break;
            }
            buf = __utf8_skip_sequence(buf);
            --offset;
        }
    } catch (...) {
        // intentionally left blank
    }
    return reinterpret_cast<char *>(buf);
}

/* */
inline static char * __utf8_offset(char *buf, unsigned int offset = -1U)
{
    return __utf8_offset(reinterpret_cast<unsigned char *>(buf), offset);
}

/* */
inline static char * __utf8_offset(const char *buf, unsigned int offset = -1U)
{
    return __utf8_offset(const_cast<char *>(buf), offset);
}

/* */
static unsigned int __utf8_length(unsigned char *buf, unsigned int count = -1U)
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
static char * __utf8_trim_left(char *buf)
{
    unsigned char *p = reinterpret_cast<unsigned char *>(buf);
    while (*p != '\0') {
        int type = __utf8_sequence_type(p);
        if (type == 1) {
            if (isspace(*p) == 0 && isprint(*p)) {
                break;
            }
        } else if (type != 0) {
            break;
        }
        try {
            p = __utf8_skip_bytes(p, type);
        } catch (...) {
            p = __utf8_skip_invalid_sequence(p);
        }
    }
    return reinterpret_cast<char *>(p);
}

/* */
static char * __utf8_trim_right(unsigned char *buf)
{
    int count = 0;
    unsigned char *p = reinterpret_cast<unsigned char *>(__utf8_offset(buf) - 1);
    while (p > buf) {
        if (*p < 0x7f) {
            if (isspace(*p) == 0 && isprint(*p)) {
                break;
            }
        } else
        if ((*p >> 6) == 0x02) {
            count++;
        } else
        if (((*p >> 5) == 0x06 && count == 1) ||
            ((*p >> 4) == 0x0e && count == 2) ||
            ((*p >> 3) == 0x1e && count == 3)) {
            break;
        } else {
            count = 0;
        }
        if (p > buf) {
            --p;
        }
    }
    try {
        return reinterpret_cast<char *>(__utf8_skip_sequence(p));
    } catch (...) {
        return reinterpret_cast<char *>(buf);
    }
}

/* */
static char * __utf8_trim_right(char *buf)
{
    return __utf8_trim_right(reinterpret_cast<unsigned char *>(buf));
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
unsigned int String::find(const char *buf, unsigned int offset) const
{
    char *p = __utf8_offset(m_buffer, offset);
    offset = p - m_buffer;
    if (buf != 0) {
        if ((p = __find(buf, offset)) != 0) {
            return __utf8_length(m_buffer, p - m_buffer);
        }
    }
    throw E_NOT_FOUND;
}

/* */
void String::trim()
{
   char *p = __utf8_trim_left (m_buffer);
   char *t = __utf8_trim_right(m_buffer);

   assign(String(p, __utf8_length(p, t - p)));
}

/* */
unsigned int String::reverse_find(const char *buf, unsigned int offset) const
{
    char *p = __utf8_offset(m_buffer, offset);
    offset = p - m_buffer;
    if (buf != 0) {
        if ((p = __reverse_find(buf, offset)) != 0) {
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
    unsigned int i;
    if (size > (m_size - offset) + 1) {
        size = (m_size - offset) + 1;
    } else {
        if (size > 0) {
            --size;
        }
    }
    for (i = 0; i < size; ++i) {
        buf[i] = m_buffer[i + offset];
    }
    buf[i] = '\0';
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
char * String::__reverse_find(const char *buf, unsigned int offset) const
{
    unsigned int i, j = 0;
    i = __utf8_offset(buf) - buf;
    if (offset < i) {
        return 0;
    }
    offset -= i;
    while (true) {
        if (buf[j] == '\0') {
            return m_buffer + offset;
        }
        if (m_buffer[offset + j] == buf[j]) {
            j++;
        } else {
            if (offset == 0) {
                break;
            }
            offset--; j = 0;
        }
    }
    return 0;
}

/* */
char * String::__find(const char *buf, unsigned int offset) const
{
    unsigned int j = 0;
    while ((offset + j) <= m_size) {
        if (buf[j] == '\0') {
            return m_buffer + offset;
        }
        if (m_buffer[offset + j] == buf[j]) {
            j++;
        } else {
            offset++; j = 0;
        }
    }
    return 0;
}

/* */
unsigned long long String::__internal_to_ulonglong() const
{
    return strtoull(m_buffer, NULL, 0);
}

/* */
long long String::__internal_to_longlong() const
{
    return strtoll(m_buffer, NULL, 0);
}

/* */
unsigned int String::__internal_to_uint() const
{
    return static_cast<unsigned int>(strtoul(m_buffer, NULL, 0));
}

/* */
int String::__internal_to_int() const
{
    return atoi(m_buffer);
}

/* */
unsigned short String::__internal_to_ushort() const
{
    return static_cast<unsigned short>(strtoul(m_buffer, NULL, 0));
}

/* */
short String::__internal_to_short() const
{
    return static_cast<short>(atoi(m_buffer));
}

/* */
unsigned char String::__internal_to_uchar() const
{
    return static_cast<unsigned char>(strtoul(m_buffer, NULL, 0));
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
