
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
#include <sys/wait.h>
#include <cerrno>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

#include <vector>

#include "exception.h"
#include "string.h"
#include "filesystem.h"
#include "utils.h"
#include "log/log.h"

using std::vector;

namespace {
    bool isFound(const String& string, unsigned int offset, const char* pattern, unsigned int& position)
    {
        try {
            position = string.find(pattern, offset);
        } catch (...) {
            return false;
        }

        return true;
    }
}

String SSI_STDERRMessage;
bool LastErrorFlag = false;

void setLastErrorMessage(const String &ErrorMessage)
{
    SSI_STDERRMessage = ErrorMessage;
    LastErrorFlag = true;
}

/**
 * @brief capture shell output as binary data
 *
 * @note	Function does not append trailing '\0'
 *
 * @return	number of bytes read or -1 for error
 */

int shell_cap(const String &s, void *buf, size_t &size)
{
    int count;
    int index = 0;
    unsigned char *buffer = reinterpret_cast<unsigned char *>(buf);
    FILE *pd = popen(s, "r");

    if (pd == NULL) {
        return -1;
    }
    do {
        count = fread(buffer + index, sizeof(unsigned char), size - index, pd);
    index += count;
    } while (count > 0);
    size = index;
    return pclose(pd);
}

/* */
int shell_cap(const String &s, String &r)
{
    char buffer[1024];
    int count;
    FILE *pd = popen(s, "r");
    if (pd == NULL) {
        return -1;
    }
    r.clear();
    do {
        count = fread(buffer, sizeof(char), sizeof(buffer)-1, pd);
        if (count > 0) {
            buffer[count] = '\0';
            r.append(buffer, count);
        }
    } while (count > 0);
    return pclose(pd);
}


static void close_parent_fds(void)
{
    DIR *dirp;
    struct dirent *d_entry;
    int fd, p_fd;

    dirp = opendir("/proc/self/fd");
    if (!dirp)
        return;

    p_fd = dirfd(dirp);
    if (p_fd == -1) {
        closedir(dirp);
        return;
    }

    while((d_entry = readdir(dirp)) != NULL) {
        if (!strcmp(d_entry->d_name, ".") ||
            !strcmp(d_entry->d_name, ".."))
            continue;
        errno = 0;
        fd = (int)strtol(d_entry->d_name, NULL, 10);
        if (errno)
            continue;
        if (p_fd == fd)
            continue;
        if (fd > 2)
            close(fd);
    }
    closedir(dirp);
}

/*
 * This function launch a shell process that executes given 's' command.
 *
 * Note:
 * This function make fork and in child process it closes all opened
 * file descriptors before execl is called.
 * It waits for child process to end.
 *
 * Returns:
 * 0 - success
 * error otherwise.
 * */
int shell(const String &s)
{
    pid_t pid;
    int ret = 0, status=0;
    String cmd = "export MDADM_EXPERIMENTAL=1; " + s;

    dlog(s);
    switch(fork()) {
    case 0:
        /* Child process.
         * Before switching into new executable close all non standard
         * file handlers.*/
        close_parent_fds();
        execl("/bin/sh", "sh", "-c", cmd.get(), NULL);
        /* If we're here then execl failed*/
        exit(-1);
        break;
    case -1:
        /* Error. Fork failed. */
        ret = -1;
        break;
    default:
        /* Parent process. */
        pid = wait(&status);
        if (pid < 0 || status)
            ret = -1;
        break;
    }
    return ret;
}

int shellEx(const String &s, unsigned int linesNum, unsigned int offset)
{
    const int ErrorCode = -1;
    const int SuccessCode = 0;
    const unsigned int BufferLength = 1024;
    const char NewLines[] = "\n";
    const unsigned int WordsToRemoveLength = 2;
    const String WordsToRemove[WordsToRemoveLength] = {
        "mdadm: ",
        "mdmon: "
    };

    FILE *in;
    String cmd = "export MDADM_EXPERIMENTAL=1; " + s + " 2>&1 1>/dev/null";
    if (!(in = popen(cmd.get(), "r"))) {
        return ErrorCode;
    }

    String rawErrorMessage;
    char buffer[BufferLength] = {};
    while (fgets(buffer, BufferLength, in) != NULL) {
        rawErrorMessage.append(buffer);
    }

    vector<String> lines;
    unsigned int pos = 0;
    unsigned int next = 0;

    String line;
    while (isFound(rawErrorMessage, pos, NewLines, next)) {
        line = String(rawErrorMessage.get(pos), next - pos);
        line.trim();

        for (unsigned int i = 0; i < WordsToRemoveLength; ++i) {
            unsigned int found = 0;

            while (isFound(line, 0, WordsToRemove[i], found)) {
                line = line.reverse_mid(line.get(found), line.get(found + WordsToRemove[i].length()));
            }
        }

        if (line != "") {
            lines.push_back(line);
        }

        pos = next + 1;
        line.clear();
    }

    String errorMessage;
    vector<String>::size_type size = lines.size();
    vector<String>::size_type i;
    for (i = (linesNum + offset) > size ? size : (linesNum + offset); i > offset; --i)
    {
        errorMessage.append(lines[size - i]);
        if (i > offset + 1) {
            errorMessage.append(" ");
        }
    }

    setLastErrorMessage(errorMessage);

    int statusCode = pclose(in);
    if (WEXITSTATUS(statusCode)) {
        return ErrorCode;
    }

    return SuccessCode;
}

/* Look if process is already running
 * */
int processExist(int pid, const String &s)
{
    File process = String("/proc/") + String(pid) + String("/cmdline");
    String name;

    try {
        process >> name;
    }
    catch (...) {
        name = "";
    }
    return name == s;
}

int readPidFile(const String &pidfilename, const String &proc)
{
    File pidfile = pidfilename;
    pid_t pid;
    try {
        pidfile >> pid;
    }
    catch (...) {
        pid = 0;
    }
    if (!processExist(pid, proc.get())) {
        pid = 0;
    }
    return pid;
}

void check_dots(String &s)
{
    String tmp = s;
    for (int i = 0; i < 3; i++) {
        tmp = tmp.after(".");
        if (tmp == "")
            s += ".0";
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
