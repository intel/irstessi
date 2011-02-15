// -*- mode: c; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
// ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

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
