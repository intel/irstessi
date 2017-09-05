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

#ifndef __EFI_H__INCLUDED__
#define __EFI_H__INCLUDED__

#include <features.h>
#include <ssi.h>

#if __GNUC_PREREQ(3, 4)
#pragma once
#endif /* __GNUC_PREREQ */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define GUID_STR_MAX    37  /* according to GUID format:
    * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" */

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
    ((struct efi_guid) \
{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
    (b) & 0xff, ((b) >> 8) & 0xff, \
    (c) & 0xff, ((c) >> 8) & 0xff, \
(d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }})

#define EFI_VAR_DIR "/sys/firmware/efi/vars"
#define EFIVARS_DIR "/sys/firmware/efi/efivars"
#define SCU_VAR "RstScuV"
#define SATA_VAR "RstSataV"
#define SSATA_VAR "RstsSatV"
#define CSATA_VAR "RstcSatV"
#define VMD_VAR "RstUefiV"

#define SATA_DEV_ID 0x2826
#define SSATA_DEV_ID 0x2827

#define VENDOR_GUID \
    EFI_GUID(0x193dfefa, 0xa445, 0x4302, 0x99, 0xd8, 0xef, 0x3a, 0xad, 0x1a, 0x04, 0xc6)

#define PATH_MAX 4096

struct efi_guid {
    __u8 b[16];
};

char *guid2str(char *buffer, struct efi_guid guid);
struct orom_info_ext * efi_get(SSI_ControllerType controllerType, unsigned int device_id);
void efi_fini(void);


#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __EFI_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
