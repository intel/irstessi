
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <asm/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "orom.h"
#include "lib/safeclib/safe_mem_lib.h"

#define VIDEO_BIOS_START_ADDR       0x000C0000U
#define SYSTEM_BIOS_START_ADDR      0x000F0000U
#define OROM_PCIR_SIGNATURE         0x52494350U     /* "PCIR" */
#define OROM_SVER_SIGNATURE         0x52455624U     /* "$VER" */
#define OROM_VALID_SIGNATURE        0x0000AA55U
#define OROM_SCAN_STEP              2048
#define OROM_SCAN_LAST              (0x000DFFFF - VIDEO_BIOS_START_ADDR)
#define OROM_REGION_SIZE            (SYSTEM_BIOS_START_ADDR - VIDEO_BIOS_START_ADDR)
#define OROM_CHUNK_SIZE             512

/* */
struct bios_hdr {
    __u16 signature;
    __u8 length;
    __u32 init_vector;
    __u8 reserved[13];
    __u16 exp_hdr_offset;
};

/* */
struct node {
    struct orom_info data;
    unsigned int orom_dev_id;
    struct node *next;
    unsigned int device_id;
};

/* */
static struct node *cache = NULL;

/* */
static _Bool bus_error = false;

/* */
static void * __orom_add_info(unsigned int device_id, void *data, unsigned int orom_dev_id)
{
    struct node *elem;

    elem = malloc(sizeof(struct node));
    if (elem) {
        elem->device_id = device_id;
        elem->orom_dev_id = orom_dev_id;
        bus_error = false;
        memcpy_s(elem, sizeof(struct node), data, sizeof(struct orom_info));
        if (!bus_error) {
            elem->next = cache;
            cache = elem;
        } else {
            free(elem);
            elem = NULL;
        }
    }
    return (void *)elem;
}

/* */
static __u8 __orom_checksum(void *start, size_t length)
{
    __u8 *p, result = 0;
    __u8 *end = (__u8 *)start + (length * OROM_CHUNK_SIZE);

    for (p = start; p < end; p++) {
        bus_error = false;
        result += *p;
        if (bus_error == true) {
            break;
        }
    }
    return (p < end) ? 1 : result;
}

/* */
static __u16 __orom_check_for_device(void *start, size_t length, unsigned int device_id)
{
    __u32 *end = (__u32 *)((__u8 *)start + (length * OROM_CHUNK_SIZE));
    __u16 vendor_id, devlist_offset, orom_dev_id;
    __u16 *devid;

    for (__u32 *p = start; p < end; p++) {
        bus_error = false;
        if (*p != OROM_PCIR_SIGNATURE || bus_error == true) {
            continue;
        }
        vendor_id = *(p + 1);
        orom_dev_id = *(p + 1) >> 16;
        devlist_offset = *(p + 2);

        if (bus_error == true) {
            continue;
        }
        if (vendor_id == INTEL_PCI_VENDOR_ID) {
            for (devid = (void *)p + devlist_offset; *devid; devid++) {
                if (*devid == device_id && orom_dev_id != 0)
                    return orom_dev_id;
            }
        }
    }
    return 0;
}

/* */
static void * __orom_check_for_data(void *start, size_t length, unsigned int device_id, unsigned int orom_dev_id)
{
    __u32 *end = (__u32 *)((__u8 *)start + (length * OROM_CHUNK_SIZE));
    void *result = NULL;

    for (__u32 *p = start; p < end; p++) {
        bus_error = false;
        if (*p == OROM_SVER_SIGNATURE && bus_error == false) {
            result = __orom_add_info(device_id, p, orom_dev_id);
            break;
        }
    }
    return result;
}

/* */
static void * __orom_check_block(struct bios_hdr *hdr, unsigned int device_id)
{
    if (hdr->signature != OROM_VALID_SIGNATURE) {
        return NULL;
    }
    if (__orom_checksum(hdr, hdr->length)) {
        return NULL;
    }
    unsigned int orom_dev_id =  __orom_check_for_device(hdr, hdr->length, device_id);
    if (orom_dev_id == 0) {
        return NULL;
    }
    return __orom_check_for_data(hdr, hdr->length, device_id, orom_dev_id);
}

/* */
static void * __orom_scan_for_device(unsigned int device_id, void *base_addr)
{
    unsigned long offset = 0;
    void *result = NULL;

    while (offset < OROM_SCAN_LAST) {
        bus_error = false;
        result = __orom_check_block((struct bios_hdr *)((__u8 *)base_addr + offset), device_id);
        if (result != NULL) {
            break;
        }
        offset += OROM_SCAN_STEP;
    }
    return result;
}

/* */
static void __orom_bus_error_handler(int signum)
{
    if (signum == SIGBUS) {
        bus_error = true;
    }
}

/* */
static void __orom_fini(void)
{
    struct node *elem;
    while (cache) {
        elem = cache->next;
        free(cache);
        cache = elem;
    }
}

/* */
static void * __orom_init(unsigned int device_id)
{
    void *result = NULL;

    if (signal(SIGBUS, __orom_bus_error_handler) == SIG_ERR) {
        return NULL;
    }
    int fd = open("/dev/mem", O_RDWR);
    if (fd >= 0) {
        void *base_addr = mmap(NULL, OROM_REGION_SIZE, PROT_READ, MAP_PRIVATE,
            fd, VIDEO_BIOS_START_ADDR);
        if (base_addr != MAP_FAILED) {
            result = __orom_scan_for_device(device_id, base_addr);
            munmap(base_addr, OROM_REGION_SIZE);
        }
        close(fd);
    }
    signal(SIGBUS, SIG_DFL);
    return result;
}

/* */
static void * __orom_get(unsigned int device_id)
{
    struct node *elem = cache;
    while (elem) {
        if (elem->device_id == device_id) {
            break;
        }
        elem = elem->next;
    }
    return (void *)elem;
}

/* */
void orom_init(void)
{
#if defined(HAVE_ATEXIT)
    if (cache == NULL) {
        atexit(__orom_fini);
    }
#endif /* HAVE_ATEXIT */
}

/* */
void orom_fini(void)
{
    __orom_fini();
}

/* */
struct orom_info_ext * orom_get(unsigned int device_id)
{
    void *result = __orom_get(device_id);
    if (result == NULL) {
        result = __orom_init(device_id);
    }
    return result;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
