/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <climits>
#include <cstdlib>

extern "C" {
#include "lib/safeclib/safe_str_lib.h"
}

#include "filesystem.h"
#include "utils.h"

/* */
void CanonicalPath::__canonicalize_path_name(const char *path)
{
    if (path == NULL) {
        throw E_NULL_POINTER;
    }
    char *p = canonicalize_file_name(path);
    assign(p);
    if (p) {
        free(p);
    }
}

/* */
void Directory::__internal_read_content()
{
    __internal_clear_content();
    struct stat st;

    DIR *pDir = opendir(m_buffer);
    if (pDir == NULL) {
        throw errno_to_exception_code(errno);
    }
    struct dirent *dirent;
    while ((dirent = readdir(pDir)) != NULL) {
        String d_name = dirent->d_name;
        if (d_name == "." || d_name == "..") {
            continue;
        }
        if (m_Filter) {
            try {
                d_name.find(m_Filter);
            } catch (...) {
                continue;
            }
        }
        String d_path = *this + d_name;
        if (stat(d_path, &st) < 0) {
            throw errno_to_exception_code(errno);
        }
        Path *pPath = NULL;
        if (S_ISREG(st.st_mode)) {
            File *pFile = new File(d_path);
            pPath = pFile;
            m_Files.push_back(pFile);
        }
        if (S_ISDIR(st.st_mode)) {
            Directory *pDirectory = new Directory(d_path);
            pPath = pDirectory;
            m_Directories.push_back(pDirectory);
        }
        if (pPath != NULL) {
            m_Content.push_back(pPath);
        }
    }
    closedir(pDir);
}

/* */
void Directory::__internal_clear_content()
{
    foreach (i, m_Content) {
        delete *i;
    }
    m_Content.clear();
    m_Files.clear();
    m_Directories.clear();
}

/* */
void Directory::__internal_copy_content(const Directory &directory)
{
    __internal_clear_content();

    foreach (i, directory.m_Files) {
        Path *pPath = new File(*(*i));
        m_Content.push_back(pPath);
        m_Files.push_back(dynamic_cast<File *>(pPath));
    }
    foreach (i, directory.m_Directories) {
        Path *pPath = new Directory(*(*i));
        m_Content.push_back(pPath);
        m_Directories.push_back(dynamic_cast<Directory *>(pPath));
    }
}

/* */
void File::read(void *buffer, unsigned int size)
{
    if (buffer == NULL) {
        throw E_NULL_POINTER;
    }
    __internal_read_content();
    if (size == 0) {
        throw E_BUFFER_TOO_SMALL;
    }
    memcpy_s(buffer, size, m_pContent, size);
}

/* */
void File::__internal_clear_content()
{
    m_ContentCapacity = 0;
    m_ContentSize = 0;
    if (m_pContent != NULL) {
        delete [] m_pContent;
        m_pContent = NULL;
    }
}

/* */
void File::__internal_realloc_content(unsigned long long size, bool copy)
{
    if (size > m_ContentCapacity) {
        unsigned char *p = new unsigned char[size + 1];
        if (m_pContent != NULL) {
            if (copy == true && m_ContentSize > 0) {
                memcpy_s(p, size, m_pContent, m_ContentSize);
            }
            delete [] m_pContent;
        }
        m_pContent = p;
        m_ContentCapacity = size;
    }
}

/* */
void File::__internal_read_from_virtual_fs(int fd)
{
    char temp[1024];
    while (true) {
        int bytes_read = ::read(fd, temp, sizeof(temp));
        if (bytes_read < 0) {
            throw errno_to_exception_code(errno);
        }
        if (bytes_read == 0) {
            break;
        }
        __internal_realloc_content(m_ContentSize + bytes_read);
        memcpy_s(m_pContent + m_ContentSize, bytes_read, temp, bytes_read);
        m_ContentSize += bytes_read;
        if (static_cast<unsigned int>(bytes_read) < sizeof(temp)) {
            break;
        }
    }
}

/* */
void File::__internal_read_from_physical_fs(int fd, unsigned long long size)
{
    if (size < SSIZE_MAX) {
        __internal_realloc_content(size, false);
        m_ContentSize = ::read(fd, m_pContent, size);
    }
}

/* */
void File::__internal_read_content()
{
    struct stat st;
    if (stat(get(), &st) < 0) {
        throw errno_to_exception_code(errno);
    }
    if (st.st_size == 0) {
        if (st.st_dev != 0 && st.st_dev != 3) {
            throw E_FILE_EMPTY;
        }
    }
    int fd = open(get(), O_RDONLY);
    if (fd < 0) {
        throw errno_to_exception_code(errno);
    }
    m_ContentSize = 0;
    if (st.st_dev != 0 && st.st_dev != 3) {
        __internal_read_from_physical_fs(fd, st.st_size);
    } else {
        __internal_read_from_virtual_fs(fd);
    }
    close(fd);
}

/* */
void File::__internal_copy_content(const File &file)
{
    m_ContentCapacity = file.m_ContentCapacity;
    m_ContentSize = file.m_ContentSize;
    memcpy_s(m_pContent, m_ContentSize, file.m_pContent, m_ContentSize);
}

/* */
unsigned long long File::__internal_to_ulonglong()
{
    if (m_pContent == NULL) {
        throw E_NULL_POINTER;
    }
    return strtoull(reinterpret_cast<char *>(m_pContent), NULL, 0);
}

/* */
long long File::__internal_to_longlong()
{
    if (m_pContent == NULL) {
        throw E_NULL_POINTER;
    }
    return strtoll(reinterpret_cast<char *>(m_pContent), NULL, 0);
}

/* */
void File::__internal_write(char *buffer, unsigned long long size, bool append)
{
    int write_flags = 0;
    if (append) {
        write_flags = (O_WRONLY | O_APPEND | O_CREAT);
    } else {
        write_flags =  (O_WRONLY | O_TRUNC | O_NONBLOCK | O_CREAT);
    }
    int fd = open(get(), write_flags, 0660);
    if (fd < 0) {
        throw errno_to_exception_code(errno);
    }
    unsigned long long bytes;
    do {
        bytes = ::write(fd, buffer, size);
    } while (bytes == -1ULL && errno == EAGAIN);
    close(fd);
}

/* */
bool File::__internal_exists()
{
    struct stat buf;
    return stat(get(), &buf) == 0;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
