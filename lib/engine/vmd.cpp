
/*
Copyright (c) 2015, Intel Corporation
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

#include <features.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <vector>

#include <ssi.h>
#include <orom/orom.h>
#include <efi/efi.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "raid_info.h"
#include "controller.h"
#include "phy.h"
#include "session.h"
#include "nvme.h"
#include "pci_header.h"
#include "nvme_phy.h"
#include "nvme_raid_info.h"
#include "utils.h"
#include "vmd.h"
#include "vmd_raid_info.h"

/* */
VMD::VMD(const String &path)
    : Controller(path)
{
    m_Name = "VMD";
    m_DomainCount = 0;
    m_EndDevicesCount = 0;
}

/* */
void VMD::discover(const String &path)
{
    const String IntelVendorTag = "0x8086";
    CanonicalPath vmdPath(path);
    m_DomainCount++;

    Directory dir("/sys/bus/pci/drivers/nvme");
    std::list<Directory *> dirs = dir.dirs();
    dirs = dir.dirs();
    foreach (i, dirs) {
        CanonicalPath nvmeDriverPath = *(*i) + "driver";
        CanonicalPath nvmePath = *(*i);
        if (nvmeDriverPath == dir) {
            File attr;
            String vendor;
            attr = *(*i) + "vendor";
            try {
                attr >> vendor;
                if (vendor != IntelVendorTag) {
                    continue;
                }
            } catch (...) {
                /* TODO log that vendor cannot be read from filesystem */
                continue;
            }
            if ((unsigned int)nvmePath.compare(vmdPath) == nvmePath.length() - vmdPath.length()) {
                NVME_Phy *pPhy = new NVME_Phy(nvmePath, m_DomainCount, m_EndDevicesCount++, this);
                attachPhy(pPhy);
                pPhy->discover();
            }
        }
    }
}

/* */
void VMD::getAddress(SSI_Address &address) const
{
    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = SSI_FALSE;
    address.sasAddress = 0ULL;
}

RaidInfo *VMD::findRaidInfo(Container <RaidInfo> &RaidInfos)
{
    /* try EFI, there's no legacy OROM for VMD */
    struct orom_info_ext *pInfo_ext = efi_get(getControllerType(), m_PciDeviceId);
    if (pInfo_ext == NULL) {
        return NULL;
    }

    orom_info *pInfo = &pInfo_ext->data;
    foreach (i,RaidInfos) {
        if ((*i)->getControllerType() == SSI_ControllerTypeVMD &&
           (*i)->m_OromDevId == pInfo_ext->orom_dev_id) {
            m_pRaidInfo = (*i);
            (*i)->attachController(this);
            return NULL;
        }
    }
    m_pRaidInfo = new VMD_RaidInfo(this,pInfo,pInfo_ext->orom_dev_id);
    return m_pRaidInfo;
}

const std::vector<CanonicalPath>& VMD::getHandledNVMEPaths()
{
	return m_HandledNVMEPaths;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
