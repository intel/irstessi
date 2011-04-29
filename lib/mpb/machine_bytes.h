
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
