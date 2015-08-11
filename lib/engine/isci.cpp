
/*
Copyright (c) 2011, Intel Corporation
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

#include <unistd.h>
#include <asm/types.h>

#include <ssi.h>
#include <orom/orom.h>
#include <efi/efi.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "controller.h"
#include "isci.h"
#include "raid_info.h"
#include "phy.h"
#include "session.h"
#include "enclosure.h"
#include "isci_phy.h"
#include "port.h"
#include "pci_header.h"
#include "isci_raid_info.h"
#include "utils.h"

/* */
ISCI::ISCI(const String &path)
    : Controller(path)
{
    m_Name = "ISCI at " + m_Path.reverse_right("0000:");
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
        } catch (Exception) {
            /* TODO: report read failure of attribtue. */
        }
        Directory phys(*(*i), "phy");
        number *= 4;
        std::list<Directory *> phys_dirs = phys.dirs();
        foreach (j, phys_dirs) {
            Phy *pPhy = new ISCI_Phy(*(*j), number++, this);
            attachPhy(pPhy);
		}
    }
    foreach (i, m_Phys)
        (*i)->discover();
    foreach (i, m_Ports)
        (*i)->discover();
}

/* */
Port * ISCI::getPortByPath(const String &path) const
{
    foreach (i, m_Ports) {
        if ((*i)->getPath() == path) {
            return (*i);
        }
    }
    return NULL;
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

RaidInfo *ISCI::findRaidInfo(Container <RaidInfo> &RaidInfos)
{
	/* first try EFI, if its failed - try legacy OROM */

    struct orom_info_ext *pInfo_ext = efi_get(getControllerType(), m_PciDeviceId);
    if (pInfo_ext == NULL) {
        pInfo_ext = orom_get(m_PciDeviceId);
    }

	if (pInfo_ext != NULL) {
		orom_info *pInfo = &pInfo_ext->data;
		foreach(i,RaidInfos){
			if ((*i)->getControllerType() == SSI_ControllerTypeSCU &&
			   (*i)->m_OromDevId == pInfo_ext->orom_dev_id) {
				m_pRaidInfo = (*i);
				(*i)->attachController(this);
				return NULL;
			}
		}
		m_pRaidInfo = new ISCI_RaidInfo(this,pInfo,pInfo_ext->orom_dev_id);
		return m_pRaidInfo;
	}
    return NULL;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
