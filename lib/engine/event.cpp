/*
Copyright (c) 2011 - 2016, Intel Corporation
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

#include <cerrno>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctime>
#include <log/log.h>

#include "event.h"

/* */
Event::Event()
    :m_semId(0)
{
}

/* */
Event::~Event()
{
    /* delete semaphore */
    if (semctl(m_semId, 0, IPC_RMID) < 0) {
        dlog("Failed to delete semaphore");
    }
}

/* */
SSI_Status Event::wait(unsigned int timeout)
{
    struct timespec t;
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;

    t.tv_sec = timeout / 1000;
    t.tv_nsec = (timeout % 1000) * 1000;

    int rv = semtimedop(m_semId, &op, 1, &t);
    if (rv == -1) {
        switch (errno) {
            case EAGAIN:
                return SSI_StatusTimeout;
            default:
                dlog("semtimedop() failed, errno:" + String(errno));
                return SSI_StatusFailed;
        }
    }
    dlog("Event recorded");
    return SSI_StatusOk;
}

bool Event::registerEvent()
{
    /* create semaphore */
    key_t key = ftok(KEY_GEN_PATH, KEY_GEN_NUM);

    if (key == (key_t)-1) {
        dlog("ftok() failed");
        return false;
    }

    m_semId = semget(key, 1, IPC_CREAT | 0600);
    if (m_semId == -1) {
        dlog("semget() failed");
        return false;
    }

    return true;
}
