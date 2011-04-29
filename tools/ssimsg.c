
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// -*- mode: c; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
// ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab:



#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H == 1 */

#include <features.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <asm/types.h>
#include <errno.h>

#if (HAVE_UNISTD_H == 1)
#include <unistd.h>
#endif /* HAVE_UNITSTD_H */

#include <ssi.h>

int main(int argc, char *argv[])
{
#if 0 /* APW */
    int msgId;
    key_t key;
    struct event_msg msg;

    log_init(LOG_DEBUG, "/var/log/ssimsg.log");
    log(LOG_DEBUG, "Message application started...\n");

    key = ftok(KEY_GENERATOR_PATH, GLOBAL_CHANNEL_NO);
    if (key == -1) {
        log(LOG_DEBUG, "**Error**: ftok() failed (errcode=%d [%s])\n",
            errno, strerror(errno));
        return 1;
    }

    msgId = msgget(key, 0);
    msg.type = 1;

    if (argc > 1) {
        snprintf(msg.text1, sizeof(msg.text1), "%s", arg[1]);
        if (argc > 2) {
            snprintf(msg.text2, sizeof(msg.text2), "%s", arg[2]);
            if (argc > 3) {
                msg.value = (__u32)atoi(arg[3]);
            }
        }
    } else {
        return 0;
    }

    if (msgsnd(msgId, &msg, sizeof(msg), 0) == -1) {
        log(LOG_DEBUG, "**Error**: message not sent!\n");
    } else {
        log(LOG_DEBUG, "Message %s sent\n", msg.text1);
    }

    log(LOG_DEBUG, "Message application exited...\n");
    log_fini();
#else /* APW */
    (void)argc;
    (void)argv;
#endif /* APW */
    return 0;
}
