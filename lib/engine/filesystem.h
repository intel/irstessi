
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

#ifndef __FILESYSTEM_H__INCLUDED__
#define __FILESYSTEM_H__INCLUDED__

/* */
class Path : public String {
public:
    Path() {
    }
    Path(const String &path)
        : String(path) {
    }
    Path(const char *path)
        : String(path) {
    }
    Path(const Path &path)
        : String(path) {
    }
};

inline Path operator + (const Path &left, const String &right) {
    return Path(left.get() + String("/") + right);
}
inline Path operator + (const Path &left, const char *right) {
    if (right == 0) {
        throw E_NULL_POINTER;
    }
    return Path(left.get() + String("/") + right);
}
inline Path operator + (const String &left, const Path &right) {
    return Path(left + String("/") + right.get());
}
inline Path operator + (const char *left, const Path &right) {
    if (left == 0) {
        throw E_NULL_POINTER;
    }
    return Path(left + String("/") + right.get());
}

class CanonicalPath : public Path {
public:
    CanonicalPath() {
    }
    CanonicalPath(const String &path) {
        __canonicalize_path_name(path.get());
    }
    CanonicalPath(const char *path) {
        __canonicalize_path_name(path);
    }
    CanonicalPath(const CanonicalPath &path)
        : Path(path) {
    }
    CanonicalPath(const Path &path) {
        __canonicalize_path_name(path.get());
    }

private:
    void __canonicalize_path_name(const char *path);
};

/* */
class File : public Path {
public:
    File(const String &path)
        : Path(path), m_pContent(0), m_ContentCapacity(0), m_ContentSize(0) {
    }
    File(const File &file)
        : Path(file), m_pContent(0), m_ContentCapacity(0), m_ContentSize(0) {
    }
    virtual ~File() {
        __internal_clear_content();
    }

public:
    File & operator = (const File &file) {
        __internal_clear_content();
        assign(file);
        m_pContent = file.m_pContent;
        m_ContentCapacity = file.m_ContentCapacity;
        m_ContentSize = file.m_ContentSize;
        return *this;
    }
    virtual void read(void *buffer, unsigned int size);
    virtual void read(long long &value) {
        __internal_read_content();
        value = __internal_to_longlong();
    }
    virtual void read(unsigned long long &value) {
        __internal_read_content();
        value = __internal_to_ulonglong();
    }
    virtual void read(String &s) {
        __internal_read_content();
        s = String(reinterpret_cast<char *>(m_pContent), m_ContentSize);
    }
    virtual void read(int &value) {
        __internal_read_content();
        value = static_cast<int>(__internal_to_longlong());
    }
    virtual void read(unsigned int &value) {
        __internal_read_content();
        value = static_cast<unsigned int>(__internal_to_ulonglong());
    }
    virtual void read(short &value) {
        __internal_read_content();
        value = static_cast<short>(__internal_to_longlong());
    }
    virtual void read(unsigned short &value) {
        __internal_read_content();
        value = static_cast<unsigned short>(__internal_to_ulonglong());
    }

    virtual void write(const String &s) {
        __internal_write(const_cast<char *>(s.get()), s.size());
    }
    virtual void write(void *buffer, unsigned int size) {
        __internal_write(reinterpret_cast<char *>(buffer), size);
    }
    virtual void write(long long value) {
        write(String(value));
    }
    virtual void write(unsigned long long value) {
        write(String(value));
    }
    virtual void write(int value) {
        write(String(value));
    }
    virtual void write(unsigned int value) {
        write(String(value));
    }
    virtual void write(short value) {
        write(String(value));
    }
    virtual void write(unsigned short value) {
        write(String(value));
    }

private:
    void __internal_clear_content();
    void __internal_copy_content(const File &file);
    void __internal_read_content();
    unsigned long long __internal_to_ulonglong();
    long long __internal_to_longlong();
    void __internal_read_from_physical_fs(int fd, unsigned long long size);
    void __internal_realloc_content(unsigned long long size, bool copy = true);
    void __internal_read_from_virtual_fs(int fd);
    void __internal_write(char *buffer, unsigned long long size);

protected:
    unsigned char *m_pContent;
    unsigned long long m_ContentCapacity;
    unsigned long long m_ContentSize;

    friend class SysfsAttr;
};

inline File & operator << (File &file, const String &s) {
    file.write(s); return file;
}
inline File & operator << (File &file, unsigned long long value) {
    file.write(value); return file;
}
inline File & operator << (File &file, long long value) {
    file.write(value); return file;
}
inline File & operator << (File &file, unsigned int value) {
    file.write(value); return file;
}
inline File & operator << (File &file, int value) {
    file.write(value); return file;
}
inline File & operator << (File &file, unsigned short value) {
    file.write(value); return file;
}
inline File & operator << (File &file, short value) {
    file.write(value); return file;
}

inline File & operator >> (File &file, String &s) {
    file.read(s); s.trim(); return file;
}
inline File & operator >> (File &file, unsigned long long &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, long long &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, unsigned int &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, int &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, unsigned short &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, short &value) {
    file.read(value); return file;
}

/* */
class SysfsAttr : public File {
public:
    SysfsAttr(const String &path = "")
        : File(path) {
    }
};

/* */
class Directory : public Path {
public:
    Directory(const String &path = "", const String &filter = "")
        : Path(path), m_Valid(false), m_Filter(filter) {
    }
    Directory(const Directory &dir)
        : Path(dir), m_Valid(dir.m_Valid), m_Filter(dir.m_Filter) {
        __internal_copy_content(dir);
    }
    ~Directory() {
        __internal_clear_content();
    }

public:
    operator Iterator<File *> () {
        __internal_update_content();
        return m_Files;
    }
    operator Iterator<Directory *> () {
        __internal_update_content();
        return m_Directories;
    }
    operator Iterator<Path *> () {
        __internal_update_content();
        return m_Content;
    }
    Directory & operator = (const String &path) {
        __internal_clear_content();
        assign(path);
        m_Filter = "";
        m_Valid = false;
        return *this;
    }
    Directory & operator = (const char *path) {
        __internal_clear_content();
        assign(path);
        m_Filter = "";
        m_Valid = false;
        return *this;
    }
    operator unsigned int () {
        return count();
    }
    unsigned int count() {
        __internal_update_content();
        return m_Content.count();
    }
    void setFilter(const String &filter, bool update = false) {
        m_Valid = false;
        m_Filter = filter;
        if (update) {
            __internal_update_content();
        }
    }

private:
    void __internal_update_content() {
        if (m_Valid == false) {
            try {
                __internal_read_content();
            } catch (...) {
                __internal_clear_content();
            }
            m_Valid = true;
        }
    }
    void __internal_read_content();
    void __internal_copy_content(const Directory &);
    void __internal_clear_content();

protected:
    List<Path *> m_Content;
    List<File *> m_Files;
    List<Directory *> m_Directories;
    bool m_Valid;
    String m_Filter;
};

/* */
class Tokenizer : public File {
public:
    Tokenizer(const String &path);
    ~Tokenizer();

private:
    List<String *> m_Tokens;
};

#endif /* __FILESYSTEM_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
