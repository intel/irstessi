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

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "object.h"
#include "context_manager.h"

/* */
SSI_Status Container::getHandles(SSI_Handle *pBuffer, SSI_Uint32 *bufferSize)
{
    SSI_Status status = SSI_StatusOk;
    if (bufferSize == 0) {
        return SSI_StatusInvalidParameter;
    }
    if (*bufferSize && pBuffer == 0) {
        return SSI_StatusInvalidParameter;
    }
    if (*bufferSize < m_Count) {
        status = SSI_StatusBufferTooSmall;
    } else {
        for (Iterator<Object *> i = first(); *i != 0; ++i, ++pBuffer) {
            *pBuffer = (*i)->getId();
        }
    }
    *bufferSize = m_Count;
    return status;
}

/* */
Object * Container::find(unsigned int id) const
{
    Iterator<Object *> i;
    for (i = first(); *i != 0 && (*i)->getId() != id; ++i) {
    }
    return *i;
}

/* */
Object * Container::remove(unsigned int id)
{
    Iterator<Object *> i;
    for (i = first(); *i != 0 && (*i)->getId() != id; ++i) {
    }
    Object *pObject = *i;
    if (pObject) {
        List<Object *>::remove(i);
    }
    return pObject;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
