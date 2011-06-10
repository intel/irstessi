
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

#ifndef __EXCEPTION_H__INCLUDED__
#define __EXCEPTION_H__INCLUDED__

/* */
enum Exception {
    E_FIRST = 0,
    E_SUCCESS = E_FIRST,
    E_GENERIC,
    E_INVALID_SCOPE,
    E_FILE_EMPTY,
    E_PERMISSION_DENIED,
    E_NOT_FOUND,
    E_BUFFER_TOO_SMALL,
    E_OUT_OF_RESOURCES,
    E_NULL_POINTER,
    E_INVALID_SEQUENCE,
    E_INVALID_OFFSET,
    E_INVALID_OPERATION,
    E_OVERFLOW,
    E_INVALID_OBJECT,
    E_INVALID_HANDLE,
    E_OUT_OF_RANGE,
    E_INVALID_DIRECTORY,
    E_NOT_AVAILABLE,
    E_INVALID_USAGE,
    E_ARRAY_CREATE_FAILED,
    E_VOLUME_CREATE_FAILED,
    E_SYSTEM_DEVICE,
    E_INVALID_STRIP_SIZE,
    E_INVALID_NAME,
    E_INVALID_RAID_LEVEL,
    E_LAST
};

Exception errno_to_exception_code(int err);

#endif /* __EXCEPTION_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
