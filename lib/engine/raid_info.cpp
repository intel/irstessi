/*
Copyright (c) 2011 - 2017, Intel Corporation
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

#include <asm/types.h>
#include <orom/orom.h>

#include "raid_info.h"
#include "controller.h"
#include "volume.h"
#include "session.h"

using boost::shared_ptr;

/* */
RaidInfo::RaidInfo(struct orom_info *pInfo) {
    m_OromDevId = 0;
    m_pInfo = pInfo;
}

/* */
void RaidInfo::getControllers(Container<Controller> &container) const
{
    container.clear();
    if (shared_ptr<Controller> controller = m_Controller.lock()) {
        container.add(controller);
    }
}

/* */
void RaidInfo::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addRaidInfo(shared_from_this());
}

/* */
String RaidInfo::getId() const
{
    if (shared_ptr<Controller> controller = m_Controller.lock()) {
        return "ra:" + controller->getPartId();
    } else {
        return "ra:" + String(getHandle());
    }
}

/* */
SSI_Status RaidInfo::getInfo(SSI_RaidInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->raidHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    pInfo->maxDisksPerArray = m_pInfo->dpa;
    pInfo->maxRaidDisksSupported = m_pInfo->tds;
    pInfo->maxVolumesPerHba = m_pInfo->vphba;
    pInfo->maxVolumesPerArray = m_pInfo->vpa;
    pInfo->raidEnabled = SSI_TRUE;
    pInfo->supportsDedicatedSpare = SSI_FALSE;
    pInfo->supportsGlobalSpare = SSI_TRUE;
    pInfo->supportsCreateFromExisting = SSI_TRUE;
    pInfo->supportsEmptyArrays = SSI_TRUE;

    return SSI_StatusOk;
}

/* */
SSI_Status RaidInfo::getRaidLevelInfo(SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }

    pInfo->stripSizesSupported = static_cast<SSI_StripSize>(m_pInfo->chk);

    switch (raidLevel) {
        case SSI_Raid0:
            pInfo->defaultStripSize = SSI_StripSize128kB;
            pInfo->supported = m_pInfo->rlc0 ? SSI_TRUE : SSI_FALSE;
            pInfo->minDisks = ssi_min(1, m_pInfo->tds);
            pInfo->maxDisks = m_pInfo->tds;
            if (shared_ptr<Controller> controller = m_Controller.lock()) {
                if (controller->getControllerType() == SSI_ControllerTypeVMD) {
                    switch (controller->getHardwareMode()) {
                        case SSI_HardwareKeyVROCPremium:
                            pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0 | SSI_Raid10 | SSI_Raid5);
                            pInfo->migrDiskAdd = static_cast<SSI_RaidLevel>(SSI_Raid10 | SSI_Raid5);
                            break;

                        case SSI_HardwareKeyVROCStandard:
                            pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0 | SSI_Raid10);
                            pInfo->migrDiskAdd = static_cast<SSI_RaidLevel>(SSI_Raid10);
                            break;

                        case SSI_HardwareKey3story:
                        default:
                            pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0);
                            pInfo->migrDiskAdd = SSI_RaidInvalid;
                            break;
                    }
                } else {
                    pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0 | SSI_Raid10 | SSI_Raid5);
                    pInfo->migrDiskAdd = static_cast<SSI_RaidLevel>(SSI_Raid10 | SSI_Raid5);
                }
            } else {
                pInfo->migrSupport = SSI_RaidInvalid;
                pInfo->migrDiskAdd = SSI_RaidInvalid;
            }
            pInfo->evenDiskCount = SSI_FALSE;
            pInfo->oddDiskCount = SSI_FALSE;
            break;

        case SSI_Raid1:
            pInfo->defaultStripSize = SSI_StripSize64kB;
            pInfo->supported = m_pInfo->rlc1 ? SSI_TRUE : SSI_FALSE;
            pInfo->minDisks = ssi_min(2, m_pInfo->tds);
            pInfo->maxDisks = ssi_min(2, m_pInfo->tds);
            pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0);
            pInfo->migrDiskAdd = SSI_RaidInvalid;
            pInfo->evenDiskCount = SSI_TRUE;
            pInfo->oddDiskCount = SSI_FALSE;
            break;

        case SSI_Raid10:
            pInfo->defaultStripSize = SSI_StripSize64kB;
            pInfo->supported = m_pInfo->rlc10 ? SSI_TRUE : SSI_FALSE;
            pInfo->minDisks = ssi_min(4, m_pInfo->tds);
            pInfo->maxDisks = ssi_min(4, m_pInfo->tds);
            pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid0);
            pInfo->migrDiskAdd = SSI_RaidInvalid;
            pInfo->evenDiskCount = SSI_TRUE;
            pInfo->oddDiskCount = SSI_FALSE;
            break;

        case SSI_Raid5:
            pInfo->defaultStripSize = SSI_StripSize64kB;
            pInfo->supported = m_pInfo->rlc5 ? SSI_TRUE : SSI_FALSE;
            pInfo->minDisks = ssi_min(3, m_pInfo->tds);
            pInfo->maxDisks = m_pInfo->tds;
            pInfo->migrSupport = static_cast<SSI_RaidLevel>(SSI_Raid5);
            pInfo->migrDiskAdd = SSI_RaidInvalid;
            pInfo->evenDiskCount = SSI_FALSE;
            pInfo->oddDiskCount = SSI_FALSE;
            break;

        case SSI_Raid6:
            pInfo->defaultStripSize = SSI_StripSize64kB;
            pInfo->supported = m_pInfo->rlc6 ? SSI_TRUE : SSI_FALSE;
            pInfo->minDisks = ssi_min(4, m_pInfo->tds);
            pInfo->maxDisks = m_pInfo->tds;
            pInfo->migrSupport = SSI_RaidInvalid;
            pInfo->migrDiskAdd = SSI_RaidInvalid;
            pInfo->evenDiskCount = SSI_FALSE;
            pInfo->oddDiskCount = SSI_FALSE;
            break;

        default:
            return SSI_StatusInvalidRaidLevel;
    }

    pInfo->created = SSI_FALSE;
    if (shared_ptr<Controller> controller = m_Controller.lock()) {
        Container<Volume> volumes;
        controller->getVolumes(volumes);

        foreach (iter, volumes) {
            Volume& volume = *(*iter);

            if (volume.getSsiRaidLevel() == raidLevel) {
                pInfo->created = SSI_TRUE;
                break;
            }
        }
    }

    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
