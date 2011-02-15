/* -*- mode: c; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */

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

#ifndef __ENDIAN_H__INCLUDED__
#define __ENDIAN_H__INCLUDED__

#define __swap16(__x)                               \
    ({ typeof(__x) x = (__x);                       \
        ((x & 0x00ffU) << 8) |                      \
        ((x & 0xff00U) >> 8) })

#define __swap32(__x)                               \
    ({ typeof(__x) x = (__x);                       \
        ((x & 0x000000ffU) << 24) |                 \
        ((x & 0xff000000U) >> 24) |                 \
        ((x & 0x0000ff00U) << 8)  |                 \
        ((x & 0x00ff0000U) >> 8)  | })

#define __swap64(__x)                               \
    ({ typeof(__x) x = (__x);                       \
        ((x & 0x00000000000000ffULL) << 56) |       \
        ((x & 0xff00000000000000ULL) >> 56) |       \
        ((x & 0x000000000000ff00ULL) << 40) |       \
        ((x & 0x00ff000000000000ULL) >> 40) |       \
        ((x & 0x0000000000ff0000ULL) << 24) |       \
        ((x & 0x0000ff0000000000ULL) >> 24) |       \
        ((x & 0x00000000ff000000ULL) <<  8) |       \
        ((x & 0x000000ff00000000ULL) >>  8) | })

#if BYTE_ORDER == LITTLE_ENDIAN

#define __cpu_to_le16(__x)      (__x)
#define __cpu_to_le32(__x)      (__x)
#define __cpu_to_le64(__x)      (__x)

#define __le16_to_cpu(__x)      (__x)
#define __le32_to_cpu(__x)      (__x)
#define __le64_to_cpu(__x)      (__x)

#define __cpu_to_be16(__x)      __swap16(__x)
#define __cpu_to_be32(__x)      __swap32(__x)
#define __cpu_to_be64(__x)      __swap64(__x)

#define __be16_to_cpu(__x)      __swap16(__x)
#define __be32_to_cpu(__x)      __swap32(__x)
#define __be64_to_cpu(__x)      __swap64(__x)

#elif BYTE_ORDER == BIG_ENDIAN

#define __cpu_to_le16(__x)      __swap16(__x)
#define __cpu_to_le32(__x)      __swap32(__x)
#define __cpu_to_le64(__x)      __swap64(__x)

#define __le16_to_cpu(__x)      __swap16(__x)
#define __le32_to_cpu(__x)      __swap32(__x)
#define __le64_to_cpu(__x)      __swap64(__x)

#define __cpu_to_be16(__x)      (__x)
#define __cpu_to_be32(__x)      (__x)
#define __cpu_to_be64(__x)      (__x)

#define __be16_to_cpu(__x)      (__x)
#define __be32_to_cpu(__x)      (__x)
#define __be64_to_cpu(__x)      (__x)

#else /* BYTE_ORDER */
#error You are trying to build for the platform of unknown endianess.
#endif /* BYTE_ORDER */

#endif /* __ENDIAN_H__INCLUDED__ */
