
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if __GNUC_PREREQ(3, 4)
#pragma once
#endif /* __GNUC_PREREQ */

#ifndef __OROM_H__INCLUDED__
#define __OROM_H__INCLUDED__

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* */
#define INTEL_PCI_VENDOR_ID             0x8086
#define PCI_CLASS_RAID_CNTRL            0x010400

/* */
struct orom_info {
    __u32 signature;
    struct {
        __u8 major;
        __u8 minor;
    } table_rev;
    struct {
        __u16 major;
        __u16 minor;
        __u16 hotfix;
        __u16 build;
    } prod_ver;
    __u8 table_len;
    __u8 table_checksum;
    union {
        __u16 rlc;
        struct {
            __u16 rlc0:1;
            __u16 rlc1:1;
            __u16 rlc10:1;
            __u16 rlc1e:1;
            __u16 rlc5:1;
            __u16 rlc_cng:1;
            __u16 rlc6:1;
            __u16 rlc_res:9;
        };
    };
    union {
        __u16 chk;
        struct {
            __u16 chk2k:1;
            __u16 chk4k:1;
            __u16 chk8k:1;
            __u16 chk16k:1;
            __u16 chk32k:1;
            __u16 chk64k:1;
            __u16 chk128k:1;
            __u16 chk256k:1;
            __u16 chk512k:1;
            __u16 chk1m:1;
            __u16 chk2m:1;
            __u16 chk4m:1;
            __u16 chk8m:1;
            __u16 chk16m:1;
            __u16 chk32m:1;
            __u16 chk64m:1;
        };
    };
    __u16 dpa;
    __u16 tds;
    __u8 vpa;
    __u8 vphba;
    union {
        __u32 attr;
        struct {
            __u32 a_rlc0:1;
            __u32 a_rlc1:1;
            __u32 a_rlc10:1;
            __u32 a_rlc1e:1;
            __u32 a_rlc5:1;
            __u32 a_rlc_cng:1;
            __u32 a_rlc6:1;
            __u32 a_rlc_reserved:9;
            __u32 a_reserved:9;
            __u32 a_nvm:1;
            __u32 a_2tb_disk:1;
            __u32 a_bbm:1;
            __u32 a_do_not_use_1:1;
            __u32 a_2tb_vol:1;
            __u32 a_do_not_use_2:1;
            __u32 a_chsum_verify:1;
        };
    };
    union {
        __u32 caps;
        struct {
            __u32 c_esata:1;
            __u32 c_turbo_memory:1;
            __u32 c_hdd_passwd:1;
            __u32 c_danbury:1;
            __u32 c_disk_coercion:1;
            __u32 c_reserved:27;
        };
    };
    union {
        __u32 features;
        struct {
            __u32 f_hdd_unlock:1;
            __u32 f_led_locate:1;
            __u32 f_ent_system:1;
            __u32 f_zpodd:1;
            __u32 f_large_dram_cache:1;
            __u32 f_rhoi:1;
            __u32 f_read_patrol:1;
            __u32 f_hardware_xor:1;
            __u32 f_reserved:24;
        };
    };
} __attribute__((packed));

struct orom_info_ext {
    struct orom_info data;
    unsigned int orom_dev_id;
};

/* */
void orom_init(void);

/* */
void orom_fini(void);

/* */
struct orom_info_ext * orom_get(unsigned int device_id);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __OROM_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
