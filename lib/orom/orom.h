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

/* */
void orom_init(void);

/* */
void orom_fini(void);

/* */
struct orom_info * orom_get(unsigned int device_id);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __OROM_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
