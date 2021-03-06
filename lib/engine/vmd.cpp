/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <vector>
#include <orom/orom.h>
#include <efi/efi.h>

#include "vmd.h"
#include "nvme_phy.h"
#include "vmd_raid_info.h"

using std::vector;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
VMD::VMD(const String &path)
    : Controller(path)
{
    m_Name = "Intel(R) VROC";
    m_EndDevicesCount = 0;
    m_PhyRemoveDisk = true;

    struct orom_info_ext *pInfo_ext = efi_get(getControllerType(), m_PciDeviceId);
    if (pInfo_ext != NULL) {
        orom_info *pInfo = &pInfo_ext->data;

        const u_int32_t NoKey = 0;
        const u_int32_t StandardKey = 1;
        const u_int32_t PremiumKey = 2;
        const u_int32_t IntelOnlyKey = 3;

        switch (pInfo->f_sku_mode) {
            case NoKey:
                m_hardwareMode = SSI_HardwareKey3story;
                m_Name += " (in pass-thru mode)";
                break;

            case StandardKey:
                m_hardwareMode = SSI_HardwareKeyVROCStandard;
                m_Name += " (Standard)";
                break;

            case PremiumKey:
                m_hardwareMode = SSI_HardwareKeyVROCPremium;
                m_Name += " (Premium)";
                break;

            case IntelOnlyKey:
                m_hardwareMode = SSI_HardwareKeyVROCIntelSSDonly;
                m_Name += " (Intel-SSD-only)";
                break;
        }

        m_supportsTpv = pInfo->f_tpv == 1;
    }
}

/* */
void VMD::discover(const String &path)
{
    CanonicalPath vmdPath(path);

    Directory dir("/sys/bus/pci/drivers/nvme");
    std::list<Directory *> dirs = dir.dirs();
    dirs = dir.dirs();
    foreach (i, dirs) {
        CanonicalPath nvmeDriverPath = *(*i) + "driver";
        CanonicalPath nvmePath = *(*i);
        if (nvmeDriverPath == dir) {
            if ((unsigned int)nvmePath.compare(vmdPath) == nvmePath.length() - vmdPath.length()) {
                shared_ptr<NVME_Phy> pPhy = shared_ptr<NVME_Phy>(new NVME_Phy(nvmePath, m_EndDevicesCount++, shared_from_this()));
                attachPhy(pPhy);
                pPhy->discover();
            }
        }
    }
}

/* */
void VMD::getAddress(SSI_Address &address) const
{
    parse_pci_address(m_Path, address);
}

shared_ptr<RaidInfo> VMD::findRaidInfo(Container <RaidInfo> &RaidInfos)
{
    /* try EFI, there's no legacy OROM for VMD */
    struct orom_info_ext *pInfo_ext = efi_get(getControllerType(), m_PciDeviceId);
    if (pInfo_ext == NULL) {
        return shared_ptr<RaidInfo>();
    }

    orom_info *pInfo = &pInfo_ext->data;
    foreach (i,RaidInfos) {
        if ((*i)->getControllerType() == SSI_ControllerTypeVMD &&
           (*i)->m_OromDevId == pInfo_ext->orom_dev_id) {
            m_pRaidInfo = (*i);
            (*i)->attachController(shared_from_this());
            return shared_ptr<RaidInfo>();
        }
    }

    shared_ptr<VMD> parent = dynamic_pointer_cast<VMD>(shared_from_this());
    m_pRaidInfo = shared_ptr<RaidInfo>(new VMD_RaidInfo(parent, pInfo, pInfo_ext->orom_dev_id));
    return m_pRaidInfo;
}

const vector<CanonicalPath>& VMD::getHandledNVMEPaths() const
{
    return m_HandledNVMEPaths;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
