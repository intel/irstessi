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
        : Path(path), m_pContent(0) {
    }
    File(const char *path)
        : Path(path), m_pContent(0) {
    }
    File(const File &file)
        : Path(file), m_pContent(0) {
    }
    virtual ~File() {
        if (m_pContent) {
            delete [] m_pContent;
        }
    }

public:
    void read(String &s);
    void read(void *buffer, unsigned int size);
    void read(long long &value);
    void read(unsigned long long &value);
    void read(int &value);
    void read(unsigned int &value);
    void read(short &value);
    void read(unsigned short &value);

    void write(String &s);
    void write(const void *buffer, unsigned int size);
    void write(long long value) {
        write(String(value));
    }
    void write(unsigned long long value) {
        write(String(value));
    }
    void write(int value) {
        write(String(value));
    }
    void write(unsigned int value) {
        write(String(value));
    }
    void write(short value) {
        write(String(value));
    }
    void write(unsigned short value) {
        write(String(value));
    }

protected:
    unsigned char *m_pContent;
    bool m_Valid;
};

/* */
inline File & operator << (File &file, const String &s) {
    file.write(s); return file;
}
inline File & operator << (File &file, unsigned int value) {
    file.write(value); return file;
}
inline File & operator << (File &file, int value) {
    file.write(value); return file;
}
inline File & operator << (File &file, unsigned long long value) {
    file.write(value); return file;
}
inline File & operator << (File &file, long long value) {
    file.write(value); return file;
}
inline File & operator << (File &file, short value) {
    file.write(value); return file;
}
inline File & operator << (File &file, unsigned short value) {
    file.write(value); return file;
}

/* */
inline File & operator >> (File &file, String &s) {
    file.read(s); return file;
}
inline File & operator >> (File &file, unsigned int &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, int &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, unsigned long long &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, long long &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, unsigned short &value) {
    file.read(value); return file;
}
inline File & operator >> (File &file, short &value) {
    file.read(value); return file;
}

/* */
class Directory : public Path {
public:
    Directory(const String &path = "", const String &filter = "")
        : Path(path), m_Valid(false), m_Filter(filter) {
    }
    Directory(const Directory &dir)
        : Path(dir), m_Valid(dir.m_Valid), m_Filter(dir.m_Filter) {
        __copy_content(dir);
    }
    ~Directory() {
        __clear_content();
    }

public:
    operator Iterator<File *> () {
        __update_content();
        return m_Files;
    }
    operator Iterator<Directory *> () {
        __update_content();
        return m_Directories;
    }
    operator Iterator<Path *> () {
        __update_content();
        return m_Content;
    }
    Directory & operator = (const String &path) {
        __clear_content();
        assign(path);
        m_Filter = "";
        m_Valid = false;
        return *this;
    }
    Directory & operator = (const char *path) {
        __clear_content();
        assign(path);
        m_Filter = "";
        m_Valid = false;
        return *this;
    }
    operator unsigned int () {
        return count();
    }
    unsigned int count() {
        __update_content();
        return m_Content.count();
    }

private:
    void __update_content() {
        if (m_Valid == false) {
            try {
                __read_content();
            } catch (...) {
                __clear_content();
            }
            m_Valid = true;
        }
    }
    void __read_content();
    void __copy_content(const Directory &);
    void __clear_content();

protected:
    List<Path *> m_Content;
    List<File *> m_Files;
    List<Directory *> m_Directories;
    bool m_Valid;
    String m_Filter;
};

#endif /* __FILESYSTEM_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
