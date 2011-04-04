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
#include "raid_info.h"
#include "session.h"

/* */
RaidInfo::RaidInfo(int disksPerArray, int totalRaidDisks, int volumesPerArray,
    int volumesPerHBA, unsigned short supportedChunkSize)
    : m_DisksPerArray(disksPerArray),
      m_RaidDisksSupported(totalRaidDisks),
      m_VolumesPerHBA(volumesPerHBA),
      m_VolumesPerArray(volumesPerArray),
      m_SupportedStripSizes(supportedChunkSize)
{
}

/* */
void RaidInfo::getControllers(Container<Controller> &container) const
{
    container = m_Controllers;
}

/* */
void RaidInfo::acquireId(Session *pSession)
{
    pSession->addRaidInfo(this);
}

/* */
SSI_Status RaidInfo::getInfo(SSI_RaidInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->raidHandle = getId();
    pInfo->maxDisksPerArray = m_DisksPerArray;
    pInfo->maxRaidDisksSupported = m_RaidDisksSupported;
    pInfo->maxVolumesPerHba = m_VolumesPerHBA;
    pInfo->maxVolumesPerArray = m_VolumesPerArray;
    pInfo->raidEnabled = SSI_TRUE;
    pInfo->supportsDedicatedSpare = SSI_TRUE;
    pInfo->supportsGlobalSpare = SSI_TRUE;
    pInfo->supportsCreateFromExisting = SSI_TRUE;
    pInfo->supportsEmptyArrays = SSI_TRUE;

    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
