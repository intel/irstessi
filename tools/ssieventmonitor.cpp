/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/inotify.h>

extern "C" {
#include "lib/safeclib/safe_str_lib.h"
}

#define MAX_LINE_LEN 1024
#define MAX_CONTAINERS	10
#define INACTIVE_STR	"inactive"
#define INACTIVE_STR_LEN 8

#ifndef SELECT_TIMEOUT
 /* default timeout: five seconds */
 #define SELECT_TIMEOUT 5
#endif

#ifndef SSIEVENTMONITOR_PIDFILE
 #define SSIEVENTMONITOR_PIDFILE "/var/run/ssieventmonitor.pid"
#endif

#ifndef MDSTAT_FILE_PATH
 #define MDSTAT_FILE_PATH "/proc/mdstat"
#endif

#ifndef INOTIFY_WATCH_DIR
 #define INOTIFY_WATCH_DIR "/dev/disk/by-path"
#endif

#ifndef SSI_MSG_NAME
 #define SSI_MSG_NAME "ssimsg"
#endif


#define SENTINEL (const char *)0

#define IS_SET(x, mask) ((x & mask) != 0)

#ifdef DEBUG
#define MSGLOG(fmt, ...) _msglog("%s():" fmt, __func__, ##__VA_ARGS__)
#else
#define MSGLOG(...)
#endif

#ifdef DEBUG
static int _msglog(const char *format, ...)
{
    va_list args;
    char buf[1024];

#if DEBUG > 1
    FILE *f = fopen("/dev/kmsg", "w+");
    if (!f)
	return 1;
#endif
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
#if DEBUG > 1
    fprintf(f, "%s\n", buf);
    fclose(f);
#endif
    fprintf(stderr, "%s\n", buf);
    return 0;
}
#endif

static int _exec_ssimsg(void)
{
    int i;
    char buffer[PATH_MAX];
    char *cp;
    const char *paths[] = {
	buffer,
	"/usr/local/bin/" SSI_MSG_NAME,
	"/usr/bin/" SSI_MSG_NAME,
	"/bin/" SSI_MSG_NAME,
	SSI_MSG_NAME,
	NULL
    };
    pid_t pid = vfork();
    int status;
    switch (pid) {
	case 0: {
	    cp = canonicalize_file_name("/proc/self/exe");
	    if (cp) {
		strcpy_s(buffer, sizeof(buffer), cp);
		free(cp);
		cp = strrchr(buffer, '/');
		if (cp)
			cp++;
		else
			cp = buffer;
		strcpy_s(cp, sizeof("ssimsg") - 1, "ssimsg");
	    }
	    for (i = 0; paths[i] != NULL; i++) {
		if (execlp(paths[i], "ssimsg", SENTINEL) < 0) {
		    continue;
		}
		perror("can't run ssimsg");
		return 1;
	    }
	    break;
	}
	case -1:
	    perror("Fork failed");
	    return 1;
	default:
	    pid = wait(&status);
	    if (pid < 0 || status != 0)
		return 1;
    }
    return 0;
}

struct dev_member {
    char *name;
    dev_member *next;
};

struct container_dev {
    dev_member *devs;
    int devnum;
};

static container_dev containers[2][MAX_CONTAINERS];
static int current = 1;

static void _free_member_devnames(dev_member *m)
{
    while(m) {
	dev_member *t = m;

	m = m->next;
	delete t->name;
	delete t;
    }
}

static int _add_member_devname(dev_member **m, char *name)
{
    dev_member *new_dev;
    char *t;

    if ((t = strchr(name, '[')) == NULL)
	/* not a device */
	return 0;

    new_dev = new dev_member();
    new_dev->name = strndup(name, t - name);
    new_dev->next = *m;
    *m = new_dev;
    return 1;
}

static void _init_containers(void)
{
    int i;

    for (current = 0; current <= 1; current++) {
	for (i = 0; i < MAX_CONTAINERS; i++) {
	    containers[current][i].devnum = -1;
	    containers[current][i].devs = NULL;
	}
    }
    current = 1;
}

static container_dev *_switch_current(void)
{
    current = 1 - current;
    return containers[current];
}

static container_dev *_get_current_container(void)
{
    return containers[current];
}

static container_dev *_get_other_container(void)
{
    return containers[1 - current];
}

static void _clear_other_container(void)
{
    int i;
    container_dev *container = _get_other_container();
    for(i = 0; i < MAX_CONTAINERS; i++) {
	if (container[i].devnum >= 0) {
	    container[i].devnum = -1;
	    _free_member_devnames(container[i].devs);
	    container[i].devs = NULL;
	}

    }
}

static int _get_devnum(char *devname)
{
    devname = strstr(devname, "md");
    if (!devname)
	return -1;
    devname += 2;
    return atoi(devname);
}

/*
  example of mdstat content

  md126 : active (auto-read-only) raid1 sda[1] sdb[0]
  156286976 blocks super external:/md127/0 [2/2] [UU]

  md127 : inactive sdb[1](S) sda[0](S)
  6306 blocks super external:imsm
*/

static int _compare_member_lists(dev_member *cm, dev_member *om)
{
    dev_member *org_om = om;
    int found;
    while(cm) {
	found = 0;
	while(om) {
	    if (strcmp(cm->name, om->name) == 0) {
		found = 1;
		break;
	    }
	    om = om->next;
	}
	if (!found)
	    return 1;
	om = org_om;
	cm = cm->next;
    }
    return 0;
}


static int _compare_members(container_dev *current,
			    container_dev *other)
{
    if (_compare_member_lists(current->devs, other->devs))
	return 1;
    return _compare_member_lists(other->devs, current->devs);
}

static int _compare_container_list(container_dev *first,
				   container_dev *second)
{
    int i, j, found = 0;

    for (i = 0; i < MAX_CONTAINERS; i++) {
	if (first[i].devnum < 0)
	    continue;
	for (j = 0; j < MAX_CONTAINERS; j++) {
	    if (second[j].devnum != first[i].devnum)
		continue;
	    found = 1;
	    if (_compare_members(first + i, second + j))
		return 1;
	    break;
	}
	if (j == MAX_CONTAINERS) /* not found */
	    return 1;
    }
    if (found || i == MAX_CONTAINERS)
	return 0;
    return 1;
}

static int _members_changed(void)
{
    if (_compare_container_list(_get_current_container(),
				_get_other_container()))
	return 1;
    return _compare_container_list(_get_other_container(),
				   _get_current_container());
}

static void _collect_devices(char *buf, container_dev *container)
{
    int i;
    for (i = 0; i < MAX_CONTAINERS; i++) {
	if (container[i].devnum < 0)
	    break;
    }
    if (i == MAX_CONTAINERS)
	return;
    container[i].devnum = _get_devnum(buf);
    buf = strstr(buf, INACTIVE_STR);
    if (buf) {
	buf += INACTIVE_STR_LEN;
    }
    while(buf++) { /* skip whitespace */
	_add_member_devname(&container[i].devs, buf);
	buf = strchr(buf, ' ');
    }
}

static int _read_mdstat(int fd)
{
    FILE *mdstat;
    char line[MAX_LINE_LEN];

    /* prepare place for current reading */
    _clear_other_container();
    /* switch sets */
    _switch_current();
    /* then read and interpret contents */
    mdstat = fdopen(dup(fd), "r");
    if (!mdstat) {
	return 1;
    }
    rewind(mdstat);
    while(fgets(line, sizeof(line) - 1, mdstat)) {
	if (!strncmp(line, "md", 2)) {
	    if (strstr(line, INACTIVE_STR)) { /* possibly container */
		char nextline[1024];
		if (fgets(nextline, sizeof(nextline) - 1, mdstat) < 0) {
		    fclose(mdstat);
		    return 1;
		}
		if (!strstr(nextline, "external:imsm"))
		    continue;
		/* get names of devices used in this container */
		_collect_devices(line, _get_current_container());
	    }
	}
    }
    fclose(mdstat);
    return 0;
}

static int _handle_mdstat(int fd)
{
    _read_mdstat(fd);
    if (_members_changed()) {
	MSGLOG("mdstat has changed!!!");
	return _exec_ssimsg();
    }
    return 0;
}

static int _get_event_count(void *buf, ssize_t read_count) {
    inotify_event *event = reinterpret_cast<inotify_event *>(buf);
    unsigned char *buffer = reinterpret_cast<unsigned char *>(buf);
    int i, j;

    j = 0;
    for (i = 0; i < read_count; ) {
	j++;
	i += (event->len + sizeof(inotify_event));
	event = reinterpret_cast<inotify_event *>(buffer + i);
    }
    return j;
}

static int _configure_inotify(int *watch_fd)
{
    int ifd;

    ifd = inotify_init();
    if (ifd < 0) {
	perror("Inotify initialization");
	return 1;
    }

    *watch_fd = inotify_add_watch(ifd, INOTIFY_WATCH_DIR, IN_CREATE | IN_DELETE);
    if (*watch_fd < 0) {
	perror("Add watch for " INOTIFY_WATCH_DIR);
	return -1;
    }

    return ifd;
}

static int _handle_inotify(int ifd, int mdstat_fd, int watch_fd)
{
    ssize_t read_count;
    char buf[(sizeof(inotify_event) + PATH_MAX) * 1];
    inotify_event *event = (inotify_event *)buf;

    int i;

    /* should not block, as we are right after pselect */
    read_count = read(ifd, buf, sizeof(buf));
    if (read_count < 0) {
	perror("read inotify handle");
	return 1;
    }
    if (read_count == 0) {
	return 0;
    }

    /* some events awaits reading */
    MSGLOG("##################################################");
    MSGLOG("%d records in buffer", _get_event_count(buf, read_count));
    if (_get_event_count(buf, read_count) < 1) {
	return 1;
    }

    /* do read events */
    i = 0;
    while(i < read_count) {
	event = (inotify_event *)(buf + i);
	if (event->wd == 0) {
	    i += sizeof(inotify_event);
	    continue;
	}
	MSGLOG("========================================================");
	MSGLOG("wd=%d mask=%u cookie=%u len=%u\n", event->wd, event->mask,
	       event->cookie, event->len);
	MSGLOG("IN_CREATE: %d, IN_DELETE: %d",
	       IS_SET(event->mask, IN_CREATE),
	       IS_SET(event->mask, IN_DELETE));
	if (event->wd == watch_fd) {
	    MSGLOG("Event for: %s", INOTIFY_WATCH_DIR);
	    _exec_ssimsg();
	    /* event here can result in change of mdstat */
	    _read_mdstat(mdstat_fd);
	    if (event->len > 0) {
		if (IS_SET(event->mask, IN_ISDIR)) {
		    MSGLOG("  Dir name: %s", event->name);
		} else {
		    MSGLOG("  File name: %s", event->name);
		}
	    }
	}
	i += event->len + sizeof(inotify_event);
    }
    return 0;
}

static void _release_inotify(int ifd, int watch_fd)
{
    inotify_rm_watch(ifd, watch_fd);
    close(ifd);
}

static int _handle_all(int stat_fd, int inot_fd, int iwatch_fd)
{
    fd_set rfds, efds;
    timespec ts = { SELECT_TIMEOUT, 0 };
    int ready, maxfd;
    maxfd = inot_fd > stat_fd ? inot_fd : stat_fd;

    while(1) {
	FD_ZERO(&rfds);
	FD_ZERO(&efds);

	FD_SET(stat_fd, &efds);
	FD_SET(inot_fd, &rfds);

	errno = 0;
	ready = 0;

	MSGLOG("Before pselect: ready=%d, inot_fd=%d, stat_fd=%d", ready,
	       FD_ISSET(inot_fd, &rfds), FD_ISSET(stat_fd, &efds));

	ready = pselect(maxfd + 1, &rfds, NULL, &efds, &ts, NULL);
	if (ready < 0) {
	    perror("pselect");
	    return 1;
	}

	MSGLOG("After pselect: ready=%d, inot_fd=%d, stat_fd=%d", ready,
	       FD_ISSET(inot_fd, &efds), FD_ISSET(stat_fd, &efds));

	if (FD_ISSET(stat_fd, &efds) || !ready)  {
	    _handle_mdstat(stat_fd);
	}
	if (FD_ISSET(inot_fd, &rfds)) {
	    _handle_inotify(inot_fd, stat_fd, iwatch_fd);
	}
    }
}

static int _event_watch(void)
{
    int stat_fd, inot_fd, iwatch_fd = -1;

    _init_containers();

    stat_fd = open(MDSTAT_FILE_PATH, O_RDONLY);
    if (stat_fd < 0)
	return 1;

    _read_mdstat(stat_fd);

    inot_fd = _configure_inotify(&iwatch_fd);
    if (inot_fd < 0) {
	close(stat_fd);
	return 1;
    }

    MSGLOG("Start of a program (inot_fd=%d, stat_fd=%d)", inot_fd, stat_fd);

    _handle_all(stat_fd, inot_fd, iwatch_fd);

    _release_inotify(inot_fd, iwatch_fd);

    close(stat_fd);

    return 0;
}

static int _write_pid_file(pid_t pid)
{
    FILE *fpid;
    fpid = fopen(SSIEVENTMONITOR_PIDFILE, "w");
    if (!fpid) {
	return 1;
    }
    fprintf(fpid, "%d\n", pid);
    fclose(fpid);
    return 0;
}

static void _signal_handler(int signal)
{
    switch(signal) {
	case SIGTERM:
	    unlink(SSIEVENTMONITOR_PIDFILE);
	    exit(0);
	default:
	    break;
    }
}

static void _setup_signals()
{
    /* ignore some signals */
    signal(SIGTSTP,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    /* */
    signal(SIGTERM, _signal_handler);
}

static void _restore_signals()
{
    /* not needed */
}

static int _deamonise(void)
{
    pid_t sid, pid = fork();
    int rv;
    if (pid < 0) {
	perror("Fork failed");
	return 1;
    } else if (pid != 0) {
	return 0;
    }
    _setup_signals();
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
	return 1;
    }
    if (!freopen( "/dev/null", "r", stdin)) {
	fclose(stdin);
    }
    if (!freopen( "/dev/null", "w", stdout)) {
	fclose(stdout);
    }
    if (!freopen( "/dev/null", "w", stderr)) {
	fclose(stderr);
    }

    if (_write_pid_file(getpid())) {
	fprintf(stderr, "Cannot write pid file (%s).\n"
		"  Anyway, daemon is starting with pid %d.\n",
		SSIEVENTMONITOR_PIDFILE, pid);
    }
    rv = _event_watch();

    /* do some cleanup */
    unlink(SSIEVENTMONITOR_PIDFILE);

    _restore_signals();
    return rv;
}

int main(int argc, char *argv[])
{
    if (getuid() != 0) {
	fprintf(stderr, "Not enough privileges. Please use root account!\n");
	return 1;
    }
    if (argc == 2 && (!strcmp(argv[1], "--daemonise") || !strcmp(argv[1], "-d"))) {
	return _deamonise();
    } else if (argc > 1) {
	fprintf(stderr, "Usage:\n"
		"    %s[ -d | --daemonise ] - starts as a daemon\n", basename(argv[0]));
        return 1;
    } else {
	return _event_watch();
    }
}
