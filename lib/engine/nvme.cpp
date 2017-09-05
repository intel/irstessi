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

#include "nvme.h"
#include "nvme_phy.h"
#include "nvme_raid_info.h"
#include "filesystem.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
NVME::NVME(const String &path)
    : Controller(path)
{
    /* TODO: read the name of controller from PCI bar */
    m_Name = "NVME at " + m_Path.reverse_right("0000:");
    m_PhyRemoveDisk = true;
}

/* */
void NVME::discover()
{
    unsigned int number = 0;
    shared_ptr<NVME_Phy> pPhy = shared_ptr<NVME_Phy>(new NVME_Phy(CanonicalPath(m_Path), number++, shared_from_this()));
    attachPhy(pPhy);
    pPhy->discover();
}

/* */
void NVME::getAddress(SSI_Address &address) const
{
    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = SSI_FALSE;
    address.sasAddress = 0ULL;
}

shared_ptr<RaidInfo> NVME::findRaidInfo(Container <RaidInfo> &RaidInfos)
{
    foreach (i, RaidInfos) {
        if ((*i)->getControllerType() == SSI_ControllerTypeNVME) {
            m_pRaidInfo = (*i);
            (*i)->attachController(shared_from_this());
            return shared_ptr<RaidInfo>();
        }
    }

    shared_ptr<NVME> parent = dynamic_pointer_cast<NVME>(shared_from_this());
    m_pRaidInfo = shared_ptr<RaidInfo>(new NVME_RaidInfo(parent));
    return m_pRaidInfo;
}
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
