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

#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "cache.h"
#include "session.h"
#include "unique_id_manager.h"
#include "session_manager.h"
#include "context_manager.h"

/* */
SessionManager::SessionManager()
    : m_pNullSession(0)
{
}

/* */
SessionManager::~SessionManager()
{
    delete m_pNullSession;

    for (Iterator<Object *> i = m_Sessions; *i != 0; ++i) {
        pContextMgr->releaseId(*i);
    }
}

/* */
Session * SessionManager::getSession(unsigned int id)
{
    Session *pSession;
    if (id == 0) {
        pSession = new Session();
        delete m_pNullSession;
        m_pNullSession = pSession;
    } else {
        pSession = dynamic_cast<Session *>(m_Sessions.find(id));
    }
    return pSession;
}

/* */
unsigned int SessionManager::openSession()
{
    Session *pSession;
    try {
        pSession = new Session();
    } catch (...) {
        return 0; /* Out of memory, no more sessions allowed. */
    }
    try {
        pContextMgr->acquireId(pSession);
        m_Sessions.add(pSession);
        return pSession->getId();
    } catch (...) {
        delete pSession;
        return 0; /* Out of resources, no more sessions allowed. */
    }
    return pSession->getId();
}

/* */
SSI_Status SessionManager::closeSession(unsigned int id)
{
    if (id == 0) {
        return SSI_StatusInvalidParameter;
    }
    Object *pSession;
    try {
        pSession = m_Sessions.remove(id);
        pContextMgr->releaseId(pSession);
    } catch (...) {
        return SSI_StatusInvalidParameter;
    }
    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
