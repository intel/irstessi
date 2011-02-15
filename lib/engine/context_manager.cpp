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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>

#include <ssi.h>
#include <log/log.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "cache.h"
#include "session.h"
#include "event.h"
#include "context_manager.h"
#include "event_manager.h"
#include "unique_id_manager.h"
#include "session_manager.h"

/* */
ContextManager::ContextManager()
    : m_pSessionMgr(0), m_pEventMgr(0), m_pUniqueIdMgr(0)
{
    m_pUniqueIdMgr = new UniqueIdManager;
    m_pEventMgr = new EventManager;
    m_pSessionMgr = new SessionManager;
}

/* */
ContextManager::~ContextManager()
{
    delete m_pSessionMgr;
    delete m_pEventMgr;
    delete m_pUniqueIdMgr;
}

/* */
SSI_Status ContextManager::getSystemInfo(SSI_SystemInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->interfaceVersionMajor = 1;
    pInfo->interfaceVersionMinor = 0;
    pInfo->libraryVersion[0] = '\0'; /* TODO */
    pInfo->maxSessions = -1U;
    pInfo->setVolCacheSizeSupport = SSI_FALSE;
    pInfo->passthroughCommandSupport = SSI_TRUE;

    return SSI_StatusOk;
}

/* */
Session * ContextManager::getSession(unsigned int id) const
{
    return m_pSessionMgr->getSession(id);
}

/* */
unsigned int ContextManager::openSession()
{
    return m_pSessionMgr->openSession();
}

SSI_Status ContextManager::closeSession(unsigned int id)
{
    return m_pSessionMgr->closeSession(id);
}

/* */
Event * ContextManager::getEvent(unsigned int id) const
{
    return m_pEventMgr->getEvent(id);
}

/* */
unsigned int ContextManager::registerEvent()
{
    return m_pEventMgr->registerEvent();
}

/* */
SSI_Status ContextManager::unregisterEvent(unsigned int id)
{
    return m_pEventMgr->unregisterEvent(id);
}

/* */
unsigned int ContextManager::acquireId(Object *pObject) {
    return m_pUniqueIdMgr->acquireId(pObject);
}

/* */
void ContextManager::releaseId(Object *pObject) {
    m_pUniqueIdMgr->releaseId(pObject);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
