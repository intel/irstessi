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

#ifndef __MPB_H__INCLUDED__
#define __MPB_H__INCLUDED__

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* MPB module error codes used by the library. */
#define MPB_E_SUCCESS               0
#define MPB_E_FAILED                1
#define MPB_E_NOT_SUPPORTED         2
#define MPB_E_OVERFLOW              3
#define MPB_E_AREA_NOT_EXIST        4
#define MPB_E_DENIED                5
#define MPB_E_DEVICE_SIZE           6
#define MPB_E_SEEK64                7
#define MPB_E_READ                  8
#define MPB_E_WRITE                 9

/* */
int mpb_assign_storage_pool(const char * path, unsigned char pool_id);

/* */
unsigned char mpb_get_storage_pool(const char * path);

/* */
int msa_write_storage_area(const char * path, void * buffer, size_t buf_size);

/* */
int msa_read_storage_area(const char * path, void * buffer, size_t buf_size);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __MPB_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
