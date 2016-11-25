/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

extern "C" {
#include <safeclib/safe_str_lib.h>
}

#include <engine/context_manager.h>

using std::nothrow;
using boost::shared_ptr;

/* */
SSI_Status SsiSessionOpen(SSI_Handle *pSession)
{
    if (pSession == NULL) {
        return SSI_StatusInvalidParameter;
    }

    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    *pSession = pContextMgr->openSession();

    if (*pSession != SSI_NULL_HANDLE) {
        return SSI_StatusOk;
    }

    return SSI_StatusInsufficientResources;
}

/* */
SSI_Status SsiSessionClose(SSI_Handle session)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    return pContextMgr->closeSession(session);
}

SSI_Status getTempSession(boost::shared_ptr<Session>& pSession)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    pSession = shared_ptr<Session>(new(nothrow) Session());
    if (!pSession) {
        return SSI_StatusInsufficientResources;
    }

    try {
        pSession->initialize();
    } catch (...) {
        return SSI_StatusFailed;
    }

    return SSI_StatusOk;
}

SSI_Status getSession(SSI_Handle session, shared_ptr<Session>& pSession)
{
    if (pContextMgr == NULL) {
        return SSI_StatusNotInitialized;
    }

    pSession = pContextMgr->getSession(session);

    if (!pSession) {
        return session == SSI_NULL_HANDLE ? SSI_StatusFailed : SSI_StatusInvalidSession;
    }

    return SSI_StatusOk;
}

SSI_Status SsiGetLastErrorMessage(SSI_Char *destination, SSI_Uint32 *num)
{
    if (num == NULL) {
        return SSI_StatusInvalidParameter;
    }

    String source;
    try {
        source = getLastErrorMessage();
    } catch (...) {
        return SSI_StatusInsufficientResources;
    }

    if (source.isEmpty()) {
        *num = 0;
        return SSI_StatusOk;
    }

    unsigned int ssiErrorLength = source.length();
    if (ssiErrorLength >= *num) {
        *num = ssiErrorLength + 1;
        return SSI_StatusBufferTooSmall;
    }

    if (destination == NULL) {
        return SSI_StatusInvalidParameter;
    }

    strncpy_s(destination, (rsize_t)*num, source.get(), ssiErrorLength);
    clearLastErrorMessage();

    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 expandtab: */
