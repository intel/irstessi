
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if __GNUC_PREREQ(3, 4)
#pragma once
#endif /* __GNUC_PREREQ */

#ifndef __STRING_H__INCLUDED__
#define __STRING_H__INCLUDED__

/**
 */
class String {
public:
    String()
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign("");
    }
    String(unsigned long long value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(long long value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(unsigned char value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(char value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(unsigned int value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(int value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(unsigned short value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(short value)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(value);
    }
    String(const String &s)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(s);
    }
    String(const String &s, unsigned int count)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(s, count);
    }
    String(const char *s, unsigned int count = -1U)
        : m_buffer(0), m_length(0), m_size(0), m_capacity(0) {
        assign(s, count);
    }
    virtual ~String() {
        delete [] m_buffer; m_length = m_size = m_capacity = 0;
    }

public:
    String & operator = (const String &s) {
        assign(s); return *this;
    }
    String & operator = (const char *buf) {
        assign(buf); return *this;
    }
    String & operator = (unsigned long long value) {
        assign(value); return *this;
    }
    String & operator = (long long value) {
        assign(value); return *this;
    }
    String & operator = (unsigned int value) {
        assign(value); return *this;
    }
    String & operator = (int value) {
        assign(value); return *this;
    }
    String & operator = (unsigned short value) {
        assign(value); return *this;
    }
    String & operator = (short value) {
        assign(value); return *this;
    }
    String & operator = (unsigned char value) {
        assign(value); return *this;
    }
    String & operator = (char value) {
        assign(value); return *this;
    }

public:
    String & operator += (const String &s) {
        append(s); return *this;
    }
    String & operator += (const char *buf) {
        append(buf); return *this;
    }
    String & operator += (unsigned long long value) {
        append(value); return *this;
    }
    String & operator += (long long value) {
        append(value); return *this;
    }
    String & operator += (unsigned int value) {
        append(value); return *this;
    }
    String & operator += (int value) {
        append(value); return *this;
    }
    String & operator += (unsigned short value) {
        append(value); return *this;
    }
    String & operator += (short value) {
        append(value); return *this;
    }
    String & operator += (unsigned char value ) {
        append(value); return *this;
    }
    String & operator += (char value) {
        append(value); return *this;
    }

public:
    operator const char * () const {
        return m_buffer;
    }
    operator bool () const {
        return isEmpty() == false;
    }
    operator unsigned long long () const {
        return __internal_to_ulonglong();
    }
    operator long long () const {
        return __internal_to_longlong();
    }
    operator unsigned int () const {
        return __internal_to_uint();
    }
    operator int () const {
        return __internal_to_int();
    }
    operator unsigned short () const {
        return __internal_to_ushort();
    }
    operator short () const {
        return __internal_to_short();
    }
    operator unsigned char () const {
        return __internal_to_uchar();
    }

public:
    const char * get(unsigned int offset = 0) const {
        if (offset > m_length) {
            throw E_INVALID_OFFSET;
        }
        return __offset(offset);
    }
    void get(char *dest, unsigned int size, unsigned int offset = 0) const {
        if (dest == NULL) {
            throw E_NULL_POINTER;
        }
        __get(dest, size, offset);
    }
    unsigned int length() const {
        return m_length;
    }
    unsigned int size() const {
        return m_size;
    }
    unsigned int capacity() const {
        return m_capacity;
    }
    bool isEmpty() const {
        return *m_buffer == '\0' || m_length == 0;
    }

public:
    void assign(const char *buf, unsigned int count = -1U);
    void assign(long long value);
    void assign(unsigned long long value);

    void assign(const String &s, unsigned int count = -1U) {
        if (&s != this) {
            assign(s.get(), count);
        }
    }
    void assign(char value) {
        assign(static_cast<long long>(value));
    }
    void assign(unsigned char value) {
        assign(static_cast<unsigned long long>(value));
    }
    void assign(int value) {
        assign(static_cast<long long>(value));
    }
    void assign(unsigned int value) {
        assign(static_cast<unsigned long long>(value));
    }
    void assign(short value) {
        assign(static_cast<long long>(value));
    }
    void assign(unsigned short value) {
        assign(static_cast<unsigned long long>(value));
    }

    void append(const String &s, unsigned int count = -1U);

    void append(const char *buf) {
        append(String(buf));
    }
    void append(unsigned long long value) {
        append(String(value));
    }
    void append(long long value) {
        append(String(value));
    }
    void append(unsigned int value) {
        append(String(value));
    }
    void append(int value) {
        append(String(value));
    }
    void append(unsigned short value) {
        append(String(value));
    }
    void append(short value) {
        append(String(value));
    }
    void append(unsigned char value) {
        append(String(value));
    }
    void append(char value) {
        append(String(value));
    }

public:
    unsigned int reverse_find(const char *buf, unsigned int offset = -1U) const;
    unsigned int find(const char *buf, unsigned int offset = 0) const;

    unsigned int reverse_find(const String &s, unsigned int offset = -1U) const {
        return reverse_find(s.m_buffer, offset);
    }
    unsigned int find(const String &s, unsigned int offset = 0) const {
        return find(s.m_buffer, offset);
    }

    String right(unsigned int length) const {
        try {
            return String(get(length));
        } catch (...) {
            return String("");
        }
    }
    String right(const char *buf) const {
        try {
            return right(find(buf));
        } catch (...) {
            return String("");
        }
    }
    String reverse_right(const char *buf) const {
        try {
            return right(reverse_find(buf));
        } catch (...) {
            return String("");
        }
    }
    String right(const String &s) const {
        return right(s.m_buffer);
    }
    String reverse_right(const String &s) const {
        return reverse_right(s.m_buffer);
    }

    String left(unsigned int length) const {
        return String(m_buffer, length);
    }
    String left(const String &s) const {
        return left(s.m_buffer);
    }
    String reverse_left(const String &s) const {
        return reverse_left(s.m_buffer);
    }
    String left(const char *buf) const {
        try {
            return left(find(buf));
        } catch (...) {
            return String("");
        }
    }
    String reverse_left(const char *buf) const {
        try {
            return left(reverse_find(buf));
        } catch (...) {
            return String("");
        }
    }

    String mid(unsigned int start, unsigned int end) const {
        try {
            if (start < end) {
                return String(get(start), end - start);
            }
        } catch (...) {
        }
        return String("");
    }
    String mid(const char *start, const char *end) const {
        try {
            unsigned int pos = find(start);
            return mid(pos, pos + find(end));
        } catch (...) {
            return String("");
        }
    }
    String mid(const String &left, const String &right) const {
        return mid(left.m_buffer, right.m_buffer);
    }

    String reverse_mid(unsigned int start, unsigned int end) const {
        try {
            if (start < end) {
                String leftPart(get(), start);
                leftPart.append(String(get(end)));
                return leftPart;
            }
        } catch (...) {
        }
        return String("");
    }
    String reverse_mid(const char *start, const char *end) const {
        try {
            unsigned int pos = find(start);
            return reverse_mid(pos, pos + find(end));
        } catch (...) {
            return String("");
        }
    }
    String reverse_mid(const String &left, const String &right) const {
        return reverse_mid(left.m_buffer, right.m_buffer);
    }

    void trim();

    String reverse_after(const String &s) const {
        try {
            return right(reverse_find(s) + s.m_length);
        } catch (...) {
            return String("");
        }
    }
    String after(const String &s) const {
        try {
            return right(find(s) + s.m_length);
        } catch (...) {
            return String("");
        }
    }

    String between(const String &s, const String &e) const {
        try {
            unsigned int t = find(s) + s.m_length;
            return mid(t, find(e, t));
        } catch (...) {
            return String("");
        }
    }
    String reverse_between(const String &s, const String &e) const {
        try {
            unsigned int t = reverse_find(e);
            return mid(reverse_find(s, t) + s.m_length, t);
        } catch (...) {
            return String("");
        }
    }

public:
    bool equal(const String &s) const {
        return this == &s || __compare(s.get()) == 0;
    }
    bool equal(const char *buf) const {
        return buf != NULL && __compare(buf) == 0;
    }
    bool different(const String &s) const {
        return this != &s && __compare(s.get()) != 0;
    }
    bool different(const char *buf) const {
        return buf != NULL && __compare(buf) != 0;
    }
    int compare(const String &s) const {
        return __compare(s.get());
    }

public:
    void clear() {
        assign("");
    }

protected:
    char *m_buffer;
    unsigned int m_length;
    unsigned int m_size;
    unsigned int m_capacity;

private:
    void __append(const char *buf, unsigned int count) {
        __copy(buf, m_length, count);
    }
    char * __reverse_find(const char *buf, unsigned int offset) const;
    char * __find(const char *buf, unsigned int offset) const;
    void __duplicate(char *buf) const;
    void __realloc(unsigned int buf_size, bool buf_copy = false);
    void __copy(const char *buf, unsigned int offset, unsigned int count);
    int __compare(const String &s, unsigned int offset = 0) const;
    void __get(char *buf, unsigned int size, unsigned int offset) const;
    char * __offset(unsigned int offset) const;

    unsigned long long __internal_to_ulonglong() const;
    long long __internal_to_longlong() const;
    unsigned int __internal_to_uint() const;
    int __internal_to_int() const;
    unsigned short __internal_to_ushort() const;
    short __internal_to_short() const;
    unsigned char __internal_to_uchar() const;
};

/* */
inline bool operator == (const String &left, const String &right) {
    return left.equal(right);
}
inline bool operator == (const String &left, const char *right) {
    return left.equal(right);
}
inline bool operator == (const char *left, const String &right) {
    String s(left);
    return s.equal(right);
}

/* */
inline bool operator != (const String &left, const String &right) {
    return left.different(right);
}
inline bool operator != (const String &left, const char *right) {
    return left.different(right);
}
inline bool operator != (const char *left, const String &right) {
    String s(left);
    return s.different(right);
}

/* */
inline String operator + (const String &left, const String &right) {
    return String(left) += right;
}
inline String operator + (const String &left, const char *right) {
    return String(left) += right;
}
inline String operator + (const char *left, const String &right) {
    return String(left) += right;
}

inline bool operator < (const String &left, const String &right) {
    return left.compare(right) < 0;
}

#endif /* __STRING_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
