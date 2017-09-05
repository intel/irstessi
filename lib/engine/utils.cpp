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

#include <cstdio>
#include <sys/wait.h>
#include <cerrno>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <log/log.h>
#include <sstream>

#include "utils.h"
#include "filesystem.h"

using std::vector;
using std::istringstream;
using std::hex;

namespace {
    String SSI_STDERRMessage;

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

/* */
void setLastErrorMessage(const String &ErrorMessage)
{
    SSI_STDERRMessage = ErrorMessage;

    /* dot removal */
    try {
        if (SSI_STDERRMessage.last() == '.') {
            SSI_STDERRMessage.pop_back();
        }
    } catch (...) {
        /* empty string */
    }
}

/* */
String getLastErrorMessage()
{
    return SSI_STDERRMessage;
}

/* */
void clearLastErrorMessage()
{
    SSI_STDERRMessage.clear();
}

/* */
void splitStringToLines(const String& output, vector<String>& lines, const String& newlines)
{
    lines.clear();

    unsigned int pos = 0;
    unsigned int next = 0;
    String line;
    while (isFound(output, pos, newlines, next)) {
        line = String(output.get(pos), next - pos);
        line.trim();

        if (line != "") {
            lines.push_back(line);
        }

        pos = next + 1;
        line.clear();
    }
}

/* */
void mdadmErrorLines(const String& output, vector<String>& lines)
{
    const String WordsToRemove[] = {
        "mdadm: ",
        "mdmon: ",
        " Aborting..."
    };
    const size_t WordsToRemoveNumber = sizeof(WordsToRemove) / sizeof(String);

    splitStringToLines(output, lines);

    vector<String>::iterator iter = lines.begin();
    while (iter != lines.end()) {
        String& line = *iter;

        for (unsigned int i = 0; i < WordsToRemoveNumber; ++i) {
            unsigned int found = 0;

            while (isFound(line, 0, WordsToRemove[i], found)) {
                line = line.reverse_mid(found, found + WordsToRemove[i].length());
            }
        }

        if (line == "") {
            iter = lines.erase(iter);
        } else {
            ++iter;
        }
    }
}

/* */
void parse_pci_address(const String& path, SSI_Address& address)
{
    try {
        String addr = path.right(path.reverse_find("/") + 1);

        unsigned int first = addr.find(":");
        String domain = addr.left(first);
        addr = addr.right(first + 1);

        unsigned int second = addr.find(":");
        String bus = addr.left(second);
        addr = addr.right(second + 1);

        unsigned int dot = addr.find(".");
        String device = addr.left(dot);

        String function = addr.right(dot + 1);

        address.bdfAddressPresent = SSI_TRUE;
        address.bdfAddress.domain = from_hex(domain);
        address.bdfAddress.bus = from_hex(bus);
        address.bdfAddress.device = from_hex(device);
        address.bdfAddress.function = from_hex(function);
    } catch (...) {
        address.bdfAddressPresent = SSI_FALSE;
        address.bdfAddress.domain = 0;
        address.bdfAddress.bus = 0;
        address.bdfAddress.device = 0;
        address.bdfAddress.function = 0;
    }

    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = SSI_FALSE;
    address.sasAddress = 0ULL;
}

/* */
unsigned int from_hex(const String& hexNumber)
{
    istringstream sin(static_cast<const char*>(hexNumber));

    unsigned int ret = 0;
    sin >> hex >> ret;
    return ret;
}

/**
 * @brief capture shell output as binary data
 *
 * @note    Function does not append trailing '\0'
 *
 * @return  number of bytes read or -1 for error
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

/* */
int shell_output(const String &command, String &output, bool errorOutput)
{
    const unsigned int BufferLength = 64 * 1024;
    const int Success = 0;
    const int Failure = -1;

    output.clear();
    FILE *in;
    const String cmd = command + (errorOutput ? " 2>&1 1>/dev/null" : " 2>/dev/null");
    if (!(in = popen(cmd.get(), "r"))) {
        return Failure;
    }

    char buffer[BufferLength] = {};
    while (fgets(buffer, BufferLength, in) != NULL) {
        output.append(buffer);
    }

    int statusCode = pclose(in);
    if (WEXITSTATUS(statusCode)) {
        return Failure;
    }

    return Success;
}

/* */
int sysfs_write(const String &message, const String &file)
{
    int fd = open(file, O_WRONLY);
    if (fd != -1) {
        size_t wcount = write(fd, static_cast<const char*>(message), message.size());
        close(fd);

        return wcount == message.size() ? 0 : 1;
    }

    return 1;
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

int shell_command(const String &cmd)
{
    String output;
    int status = shell_output(cmd, output, true);

    return status;
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
    int i = 0;
    int ret = 0, status=0;
    String cmd = "export MDADM_EXPERIMENTAL=1; " + s;
    const char *envp[] = { "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
    const char *argv[] = { "sh", "-c", cmd.get(), NULL };

    dlog(s);
    switch(fork()) {
    case 0:
        /* Child process.
         * Before switching into new executable close all non standard
         * file handlers.*/
        close_parent_fds();

        i = open("/dev/null", O_RDWR); /* bind stdin to process */
        if (i != -1) {
            if (dup(i) == -1) /* stdout */
            {
                dlog("Warning: Could not duplicate stdout.");
            }
            if (dup(i) == -1) /* stderr */
            {
                dlog("Warning: Could not duplicate stderr.");
            }
        }

        execve(argv[0], (char **)argv, (char **)envp);
        /* If we're here then execve failed*/
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
    const String cmd = "export MDADM_EXPERIMENTAL=1; " + s;
    String output;
    int status = shell_output(cmd, output, true);
    if (status != 0) {
        vector<String> lines;
        mdadmErrorLines(output, lines);

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
    }

    return status;
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
        if (tmp == "") {
            s += ".0";
        }
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
