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

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdlib>

#include "exception.h"
#include "list.h"
#include "string.h"
#include "filesystem.h"

/* */
void CanonicalPath::__canonicalize_path_name(const char *path)
{
    if (path == 0) {
        throw E_NULL_POINTER;
    }
    char *p = canonicalize_file_name(path);
    assign(p);
    if (p) {
        free(p);
    }
}

/* */
void Directory::__read_content()
{
    struct dirent *dirent;
    struct stat st;

    __clear_content();

    DIR *pDir = opendir(m_buffer);
    if (pDir == 0) {
        throw errno_to_exception_code(errno);
    }
    while ((dirent = readdir(pDir)) != 0) {
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
        Path *pPath = 0;
        if (S_ISREG(st.st_mode)) {
            File *pFile = new File(d_path);
            pPath = pFile;
            m_Files.add(pFile);
        }
        if (S_ISDIR(st.st_mode)) {
            Directory *pDirectory = new Directory(d_path);
            pPath = pDirectory;
            m_Directories.add(pDirectory);
        }
        if (pPath != 0) {
            m_Content.add(pPath);
        }
    }
    closedir(pDir);
}

/* */
void Directory::__clear_content()
{
    for (Iterator<Path *> i = m_Content; *i != 0; ++i) {
        delete *i;
    }
    m_Content.clear();
    m_Files.clear();
    m_Directories.clear();
}

/* */
void Directory::__copy_content(const Directory &directory)
{
    __clear_content();

    for (Iterator<Directory *> i = directory.m_Directories; *i != 0; ++i) {
        Path *pPath = new Directory(*(*i));
        m_Content.add(pPath);
        m_Directories.add(dynamic_cast<Directory *>(pPath));
    }
    for (Iterator<File *> i = directory.m_Files; *i != 0; ++i) {
        Path *pPath = new File(*(*i));
        m_Content.add(pPath);
        m_Files.add(dynamic_cast<File *>(pPath));
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
