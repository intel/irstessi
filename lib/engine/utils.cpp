
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
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "exception.h"
#include "list.h"
#include "string.h"
#include "filesystem.h"
#include "utils.h"
#include "log/log.h"

/* */
int shell_cap(const String &s, String &r)
{
    char buffer[1024];
    int count;
    FILE *pd = popen(s, "r");
    dlog(s);
    if (pd == 0) {
        return -1;
    }
    r.clear();
    do {
        count = fread(buffer, sizeof(char), sizeof(buffer), pd);
        if (count > 0) {
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

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
