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

#include "ahci.h"

#include "filesystem.h"
#include "ahci_phy.h"
#include "ahci_raid_info.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
AHCI::AHCI(const String &path)
    : Controller(path)
{
    /* TODO: read the name of controller from PCI bar */
    m_Name = "AHCI at " + m_Path.reverse_right("0000:");
}

/* */
void AHCI::discover()
{
    unsigned int number = 0;
    std::vector<Directory> hosts;

    Directory dir(m_Path, "host");

    if (dir.count() > 0) {
        std::list<Directory *> dirs = dir.dirs();
        foreach (i, dirs) {
            hosts.push_back(**i);
        }
    } else {
        dir = Directory(m_Path, "ata");
        std::list<Directory *> dirs = dir.dirs();
        foreach (i, dirs) {
            Directory port_dir(**i, "host");
            hosts.push_back(**port_dir.dirs().begin());
        }
    }

    foreach (i, hosts) {
        CanonicalPath temp = *i + "scsi_host";
        if (temp) {
            shared_ptr<AHCI> shared = dynamic_pointer_cast<AHCI>(shared_from_this());
            shared_ptr<AHCI_Phy> pPhy = shared_ptr<AHCI_Phy>(new AHCI_Phy(CanonicalPath(*i), number++, shared));
            attachPhy(pPhy);
            pPhy->discover();
        }
    }
}

/* */
void AHCI::getAddress(SSI_Address &address) const
{
    parse_pci_address(m_Path, address);
}

shared_ptr<RaidInfo> AHCI::findRaidInfo(Container <RaidInfo> &RaidInfos)
{
    /* first try EFI, if its failed - try legacy OROM */
    if (((m_ClassId << 16) | (m_SubClassCode << 8) | m_prgIface )== PCI_CLASS_RAID_CNTRL) {
        struct orom_info_ext *pInfo_ext = efi_get(getControllerType(), m_PciDeviceId);
        if (pInfo_ext == NULL) {
            pInfo_ext = orom_get(m_PciDeviceId);
        }

        if (pInfo_ext != NULL) {
            orom_info *pInfo = &pInfo_ext->data;
            foreach (i, RaidInfos) {
                if ((*i)->getControllerType() == SSI_ControllerTypeAHCI && (*i)->m_OromDevId == pInfo_ext->orom_dev_id) {
                    m_pRaidInfo = (*i);
                    (*i)->attachController(shared_from_this());
                    return shared_ptr<RaidInfo>();
                }
            }
            shared_ptr<AHCI> shared = dynamic_pointer_cast<AHCI>(shared_from_this());
            m_pRaidInfo = shared_ptr<RaidInfo>(new AHCI_RaidInfo(shared, pInfo, pInfo_ext->orom_dev_id));
            return m_pRaidInfo;
        }
    }

    return shared_ptr<RaidInfo>();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
