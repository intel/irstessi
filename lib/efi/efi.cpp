
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
    struct orom_info *data;
    struct node *next;
    SSI_ControllerType controllerType;
};

/* */
static struct node *efi_cache = 0;

/* */
static struct orom_info *__efi_add_info(SSI_ControllerType controllerType, struct orom_info *data)
{
    struct node *elem;

    elem = new struct node;
    if (elem) {
        elem->controllerType = controllerType;
        elem->data = data;
        elem->next = efi_cache;
        efi_cache = elem;
    } else {
        delete data;
        data = 0;
    }
    return data;
}

/* */
struct orom_info *__read_efi_variable(SSI_ControllerType controllerType)
{
    unsigned int size = 0;
    struct orom_info *data = 0;
    unsigned int var_size = sizeof(struct orom_info);
    Directory dir(EFI_VAR_DIR);
    Path var_path = "";
    char buf[GUID_STR_MAX];

    guid2str(buf, VENDOR_GUID);
    dir.setFilter((controllerType == SSI_ControllerTypeAHCI)?AHCI_VAR:SCU_VAR);
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
        return 0;
    try {
        SysfsAttr attr = var_path + "size";
        attr >> size;
    } catch (...) {
    }
    if (size != var_size)
        return 0;
    data = new struct orom_info;
    try {
        SysfsAttr attr = var_path + "data";
        attr.read(data, size);
    } catch (...) {
        delete data;
        return 0;
    }
    return data;
}

/* */
static struct orom_info * __efi_init(SSI_ControllerType controllerType)
{
    struct orom_info *result = __read_efi_variable(controllerType);
    if (result != 0)
        result = __efi_add_info(controllerType, result);
    return result;
}

/* */
static struct orom_info * __efi_get(SSI_ControllerType controllerType)
{
    struct node *elem = efi_cache;
    while (elem) {
        if (elem->controllerType == controllerType) {
            break;
        }
        elem = elem->next;
    }
    return elem?elem->data:0;
}

/* */
static void __efi_fini(void)
{
    struct node *elem;
    while (efi_cache) {
        elem = efi_cache->next;
        delete efi_cache->data;
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
struct orom_info * efi_get(SSI_ControllerType controllerType)
{
    struct orom_info *result = __efi_get(controllerType);
    if (result == 0) {
        result = __efi_init(controllerType);
    }
    return result;
}

char *guid2str(char *buffer, struct efi_guid guid)
{
    sprintf(buffer, "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
            "%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid.b[3], guid.b[2], guid.b[1], guid.b[0],
            guid.b[5], guid.b[4], guid.b[7], guid.b[6],
            guid.b[8], guid.b[9], guid.b[10], guid.b[11],
            guid.b[12], guid.b[13], guid.b[14], guid.b[15]);
    return buffer;
}

/* ex: tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
