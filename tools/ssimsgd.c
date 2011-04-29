
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


 
#if 0 /* APW */

#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H == 1 */

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <limits.h>
#include <asm/types.h>

#define LOG_FILE    "/var/log/ssimsgd.log"
#define MDMSGD_PID_FILE "/var/run/ssimsgd.pid"

#define STATUS_FILE_EXIST   2

int fd, msgId;
pid_t mdmonPid;
struct event_obj *eventObjTail;

char *supportedMessages[] = {
    "Add Channel",
    "DegradedArray",
    "SparesMissing",
    "RebuildStarted",
    "TestMessage",
    "NewArray",
    "MoveSpare",
    "RebuildStarted",
    "Rebuild20",
    "Rebuild40",
    "Rebuild60",
    "Rebuild80",
    "DeviceDisappeared",
    "RebuildFinished",
    "Fail",
    "FailSpare",
    "SpareActive",
    NULL
};

int isItSupportedMessage(const char *msg)
{
    int index;

    /* go through the list to find message id */
    for (index = 0; supportedMessages[index]; index++) {
        if (strlen(msg) == strlen(supportedMessages[index])) {
            if (strncmp(msg, supportedMessages[index], strlen(supportedMessages[index])) == 0) {
                return index;
            }
        }
    }
    return -1;
}

void processMessage(struct event_msg *msg)
{
    int msgType;
    int result;
    struct event_obj *eventObjTmp;

    log(LOG_NORMAL, "Message received: %s", msg->text1);
    if (eventObjTail == NULL) {
        log(LOG_NORMAL, "**Error**: no objects registered!");
        return;
    }

    if ((msgType = isItSupportedMessage(msg->text1)) >= 0) {
        if (msgType == 0) { /* case of 'Add Channel' message */
            log(LOG_NORMAL, "%s %d", supportedMessages[msgType], msg->value);
            if (event_register(&eventObjTail, msg->value) == 0) {
                eventObjTail->handle = msg->value;
            }
        } else if (msgType == 1) { /* case of 'Delete Channel' message */
            log(LOG_NORMAL, "%s %d", supportedMessages[msgType], msg->value);
            event_unregister(&eventObjTail, msg->value);
        } else { /* for any other notification */
            log(LOG_NORMAL, "Broadcasting event...");
            /* we receive an event */
            /* so we need to put it into registered queues */
            eventObjTmp = eventObjTail;
            while (eventObjTmp) {
                struct sembuf sem_post = { 0, 1, IPC_NOWAIT };
                log(LOG_NORMAL, "Post semaphore %u", eventObjTmp->channel_id);
                result = semop(eventObjTmp->channel_id, &sem_post, 1);
                if (result < 0) {
                    if (errno == EINVAL) {
                        log(LOG_DETAIL, "**Error**: channel (%d) does not exist!",
                               eventObjTmp->channel_id);
                    } else {
                        log(LOG_NORMAL, "**Error**: semop failed (errno=%d)", errno);
                    }
                }
                eventObjTmp = eventObjTmp->next;
            }
            log(LOG_NORMAL, "Broadcasting finished");
        }
    } else {
        log(LOG_NORMAL, "Unknown message");
    }
}

int registerPid(void)
{
    FILE *pid = fopen(MDMSGD_PID_FILE, "w+");
    if (pid) {
        fprintf(pid, "%d\n", getpid());
        fflush(pid);
        fclose(pid);
    }
    return 0;
}

int unregisterPid(void)
{
    return unlink(MDMSGD_PID_FILE);
}

static void signalHandler(int signum)
{
    (void)signum;
    /* Take appropriate actions for signal delivery */

    /* log some information */
    log(LOG_DETAIL, "Server process is going to be stopped");
    /* remove file from /var/run */
    if (unregisterPid() < 0) {
        log(LOG_DEBUG, "**Warning**: unable to remove PID file (errno=%d)",
               errno);
    }
    /* delete global channel */
    if (msgctl(msgId, IPC_RMID, NULL) == -1) {
        log(LOG_DETAIL, "Msg ctl error!");
    }

    /* stop mdmon ??? */
    kill(mdmonPid, SIGKILL);
    /* stop logging */
    log_fini();
    exit(0);
}

#define START_MONITOR_CMD "mdadm --monitor --daemonize -p ssimsgd --scan"

void runServer(void)
{
    struct event_msg msg;
    struct sigaction signalAction;
    FILE *pd;
    char cmdBuffer[4096];
    int result;

    /* Initialize logging */
    log_init(LOG_DEBUG, LOG_FILE);
    log(LOG_NORMAL, "Server start");

    /* write process id to the file */
    if (registerPid() < 0 && errno == EEXIST) {
        log(LOG_NORMAL, "Server is running!");
        return;
    }

    /* start mdadm monitor */
    if ((pd = popen(START_MONITOR_CMD, "r")) != NULL) {
        result = fread(cmdBuffer, 1, 4096, pd);
        if (pclose(pd) < 0 || result < 0) {
            log(LOG_NORMAL, "**Fatal**: unable to start monitor (errno=%d)!", errno);
            return;
        }
    }
    mdmonPid = atoi(cmdBuffer);
    log(LOG_NORMAL, "Monitor started (pid:%d)", mdmonPid);

    /* Mark events list empty */
    eventObjTail = NULL;
    msgId = event_get_global_channel();

    /* assign action to signals */
    signalAction.sa_handler = signalHandler;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &signalAction, NULL);
    sigaction(SIGINT, &signalAction, NULL);
/*  sigaction (SIGSEGV, &signalAction, NULL); */
/*  sigaction (SIGBUS, &signalAction, NULL); */
/*  sigaction (SIGABRT, &signalAction, NULL); */
/*  sigaction (SIGPIPE, &signalAction, NULL); */
/*  sigaction (SIGINT, &signalAction, NULL); */
/*  sigaction (SIGILL, &signalAction, NULL); */

    while (1) {
        /* wait for the message in global queue */
        while (msgrcv(msgId, &msg, sizeof(msg), 0, MSG_NOERROR) < 0);
        processMessage(&msg);

    }
}

#endif /* APW */

int main(int argc, char *argv[])
{
#if 0 /* APW */
    int serverRunning = 0;
    pid_t pid;
    struct stat buff;

    log(LOG_NORMAL, "Caller started!");

    /* check if server is running */
    if (stat(MDMSGD_PID_FILE, &buff) == 0) {
        log(LOG_DEBUG, "Server  already started");
        serverRunning = 1;
    } else {
        log(LOG_DEBUG, "Server will be forked");
        serverRunning = 0;
    }

    /* if server is called with some arguments (notification) */
    /* input arguments should be stored in message queue */
    if (argc > 1) {
        event_send(arg[1], 0);
    }
    if (serverRunning == 0) {
        pid = fork();
        switch (pid) {
            case -1 : /* fork error */
                log(LOG_NORMAL, "fork failed");
                break;
            case 0  : /*child process */
                log(LOG_NORMAL, "Message server started");
                runServer();
                log(LOG_NORMAL, "Message server exit");
                return 0;
            default : /* parent */
                break;
        }
    }

    log(LOG_NORMAL, "Caller exit!");
#else /* APW */
    (void)argc;
    (void)argv;
#endif /* APW */
    return 0;
}
