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

#include <unistd.h>
#include <asm/types.h>
#include <orom/orom.h>
#include <efi/efi.h>

#include "isci.h"
#include "filesystem.h"
#include "isci_phy.h"
#include "port.h"
#include "isci_raid_info.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
ISCI::ISCI(const String &path)
    : Controller(path)
{
    m_Name = "ISCI at " + m_Path.reverse_right("0000:");
    m_Address.sasAddress = 0;
    m_Address.sasAddressPresent = SSI_FALSE;
    m_Address.bdfAddress.bus = 0;
    m_Address.bdfAddress.device = 0;
    m_Address.bdfAddress.domain = 0;
    m_Address.bdfAddress.function = 0;
    m_Address.bdfAddressPresent = SSI_FALSE;
}

/* */
void ISCI::discover()
{
    Directory dir(m_Path, "host");
    File attr;
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        unsigned int number = 0;
        try {
            attr = *(*i) + "scsi_host" + (*i)->reverse_after("/") + "isci_id";
            attr >> number;
        } catch (...) {
            /* TODO: report read failure of attribtue. */
        }
        Directory phys(*(*i), "phy");
        number *= 4;
        std::list<Directory *> phys_dirs = phys.dirs();
        foreach (j, phys_dirs) {
            attachPhy(shared_ptr<Phy>(new ISCI_Phy(*(*j), number++, shared_from_this())));
        }
    }

    foreach (i, m_Phys) {
        (*i)->discover();
    }

    foreach (i, m_Ports) {
        (*i)->discover();
    }
}

/* */
shared_ptr<Port> ISCI::getPortByPath(const String &path) const
{
    foreach (i, m_Ports) {
        if ((*i)->getPath() == path) {
            return (*i);
        }
    }
    return shared_ptr<Port>();
}

/* */
void ISCI::getAddress(SSI_Address &address) const
{
    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = m_Address.sasAddressPresent;
    address.sasAddress = m_Address.sasAddress;
}

/* */
void ISCI::setAddress(SSI_Address &address)
{
    m_Address.scsiAddress.host = 0;
    m_Address.scsiAddress.bus = 0;
    m_Address.scsiAddress.target = 0;
    m_Address.scsiAddress.lun = 0;
    m_Address.sasAddressPresent = address.sasAddressPresent;
    m_Address.sasAddress = address.sasAddress;
}

shared_ptr<RaidInfo> ISCI::findRaidInfo(Container <RaidInfo> &RaidInfos)
{
    /* first try EFI, if its failed - try legacy OROM */

    struct orom_info_ext *pInfo_ext = efi_get(getControllerType(), m_PciDeviceId);
    if (pInfo_ext == NULL) {
        pInfo_ext = orom_get(m_PciDeviceId);
    }

    if (pInfo_ext != NULL) {
        orom_info *pInfo = &pInfo_ext->data;
        foreach (i, RaidInfos) {
            if ((*i)->getControllerType() == SSI_ControllerTypeSCU && (*i)->m_OromDevId == pInfo_ext->orom_dev_id) {
                m_pRaidInfo = (*i);
                (*i)->attachController(shared_from_this());
                return shared_ptr<RaidInfo>();
            }
        }

        shared_ptr<ISCI> parent = dynamic_pointer_cast<ISCI>(shared_from_this());
        m_pRaidInfo = shared_ptr<RaidInfo>(new ISCI_RaidInfo(parent, pInfo, pInfo_ext->orom_dev_id));
        return m_pRaidInfo;
    }

    return shared_ptr<RaidInfo>();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
