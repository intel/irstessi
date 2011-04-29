
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
