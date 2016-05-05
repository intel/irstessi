
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

#include <asm/types.h>
#include <cstdio>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "include/ssi.h"
#include "lib/engine/exception.h"
#include "lib/engine/container.h"
#include "lib/engine/string.h"
#include "lib/efi/efi.h"
#include "lib/orom/orom.h"
#include "lib/engine/filesystem.h"
#include "lib/log/log.h"
#include "lib/engine/utils.h"

/* */
struct node {
    struct orom_info_ext *orom_ext;
    struct node *next;
    unsigned int device_id;

};

/* */
static struct node *efi_cache = NULL;

/* */
static struct orom_info_ext *__efi_add_info(struct orom_info *data, unsigned int orom_dev_id, unsigned int device_id)
{
    struct node *elem;
    struct orom_info_ext *orom_ext;

    elem = new struct node;
    orom_ext = new struct orom_info_ext;
    if (elem && orom_ext) {
        orom_ext->orom_dev_id = orom_dev_id;
        orom_ext->data = *data;
        elem->device_id = device_id;
        elem->next = efi_cache;
        elem->orom_ext = orom_ext;
        efi_cache = elem;
    } else {
        delete data;
        data = NULL;
        delete orom_ext;
        orom_ext = NULL;
        delete elem;
        elem = NULL;
        return NULL;
    }
    return elem->orom_ext;
}

struct orom_info *__read_efi_var(String var_name)
{
    struct orom_info *data = NULL;
    int var_size = sizeof(struct orom_info);
    Directory dir(EFIVARS_DIR);
    String Filter = var_name;
    char var_path[PATH_MAX] = "";
    char buf[GUID_STR_MAX];
    int fd, n;

    guid2str(buf, VENDOR_GUID);

	snprintf(var_path, PATH_MAX, "%s/%s-%s", EFIVARS_DIR, (const char*) var_name, guid2str(buf, VENDOR_GUID));

	fd = open(var_path, O_RDONLY);
	if (fd < 0)
		return NULL;

	/* read the variable attributes and ignore it */
	n = read(fd, buf, sizeof(__u32));
	if (n < 0) {
		close(fd);
		return NULL;
	}

	/* read the variable data */
    data = new struct orom_info;
	n = read(fd, data, var_size);
	close(fd);
	if (n < var_size) {
		delete data;
		return NULL;
	}

	return data;
}

/* */
struct orom_info *__read_efi_variable(String var_name)
{
    unsigned int size = 0;
    struct orom_info *data = NULL;
    unsigned int var_size = sizeof(struct orom_info);
    Directory dir(EFI_VAR_DIR);
    String Filter = var_name;
    Path var_path = "";
    char buf[GUID_STR_MAX];

    /* First try read efivars (old interface), if fails - try efi/variables/ */
    data = __read_efi_var(var_name);
    if (data)
        return data;

    guid2str(buf, VENDOR_GUID);
    dir.setFilter(Filter);

    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        try {
            (*i)->find(buf);
            var_path = *(*i);
            break;
        } catch (...) {
            continue;
        }
    }
    if (var_path  == "")
        return NULL;
    try {
        File attr = var_path + "size";
        attr >> size;
    } catch (...) {
    }
    if (size != var_size)
        return NULL;
    data = new struct orom_info;
    try {
        File attr = var_path + "data";
        attr.read(data, size);
    } catch (...) {
        delete data;
        return NULL;
    }
    return data;
}

/* */
static struct orom_info_ext * __efi_init(SSI_ControllerType controllerType, unsigned int device_id)
{
    struct orom_info *orom_data = NULL;
    unsigned int orom_dev_id;
    if (controllerType == SSI_ControllerTypeSCU) {
        orom_data = __read_efi_variable(SCU_VAR);
        orom_dev_id = device_id;
    } else if (controllerType == SSI_ControllerTypeAHCI) {
        switch (device_id) {
        case SATA_DEV_ID:
            orom_data = __read_efi_variable(SATA_VAR);
            orom_dev_id = SATA_DEV_ID;
            break;
        case SSATA_DEV_ID:
            orom_data = __read_efi_variable(SSATA_VAR);
            orom_dev_id = SSATA_DEV_ID;
            break;
        }
        if (orom_data == NULL) {
            orom_data = __read_efi_variable(CSATA_VAR);
            orom_dev_id = SATA_DEV_ID;
        }
    } else if (controllerType == SSI_ControllerTypeVMD) {
            orom_data = __read_efi_variable(VMD_VAR);
            orom_dev_id = device_id;
    }

    if (orom_data != NULL) {
        struct orom_info_ext *result = __efi_add_info(orom_data, orom_dev_id, device_id);
        return result;
    }
    return NULL;
}

/* */
static struct orom_info_ext * __efi_get(unsigned int device_id)
{
    struct node *elem = efi_cache;
    while (elem) {
        if (elem->device_id == device_id) {
            break;
        }
        elem = elem->next;
    }
    return elem?elem->orom_ext:NULL;
}

/* */
static void __efi_fini(void)
{
    struct node *elem;
    while (efi_cache) {
        elem = efi_cache->next;
        delete &efi_cache->orom_ext->data;
        delete efi_cache;
        efi_cache = elem;
    }
}

/* */
void efi_fini(void)
{
    __efi_fini();
}

/* */
struct orom_info_ext * efi_get(SSI_ControllerType controllerType, unsigned int device_id)
{
    struct orom_info_ext *result = __efi_get(device_id);
    if (result == NULL) {
        result = __efi_init(controllerType, device_id);
    }
    return result;
}

char *guid2str(char *buffer, struct efi_guid guid)
{
    snprintf(buffer, sizeof(guid.b), "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
            "%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid.b[3], guid.b[2], guid.b[1], guid.b[0],
            guid.b[5], guid.b[4], guid.b[7], guid.b[6],
            guid.b[8], guid.b[9], guid.b[10], guid.b[11],
            guid.b[12], guid.b[13], guid.b[14], guid.b[15]);
    return buffer;
}

/* ex: tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
