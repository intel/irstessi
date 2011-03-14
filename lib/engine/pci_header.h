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

#ifndef __PCI_HEADER_H__INCLUDED__
#define __PCI_HEADER_H__INCLUDED__

/* */
struct PCIHeader {
    __u16 vendorId;
    __u16 deviceId;
    __u16 command;
    __u16 status;
    __u8 revisionId;
    __u8 prgIface;
    __u8 subClassId;
    __u8 classId;
    __u8 bist;
    __u8 headerType;
    __u8 latency;
    __u8 cacheLineSize;
    __u32 bar0;
    __u32 bar1;
    __u32 bar2;
    __u32 bar3;
    __u32 bar4;
    __u32 bar5;
    __u32 cardBusPtr;
    __u16 subSystemVendorId;
    __u16 subSystemId;
};

#endif /* __PCI_HEADER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
