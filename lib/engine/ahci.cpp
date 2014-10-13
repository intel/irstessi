
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
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "raid_info.h"
#include "controller.h"
#include "phy.h"
#include "session.h"
#include "ahci.h"
#include "pci_header.h"
#include "ahci_phy.h"
#include "ahci_raid_info.h"

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
        for (Iterator<Directory *> i = dir; *i != 0; ++i)
            hosts.push_back(**i);
    } else {
        dir = Directory(m_Path, "ata");

        for (Iterator<Directory *> i = dir; *i != 0; ++i) {
            Directory port_dir(**i, "host");
            hosts.push_back(**static_cast<Iterator<Directory *> >(port_dir));
        }
    }

    for (std::vector<Directory>::iterator i = hosts.begin(); i != hosts.end(); ++i) {
        CanonicalPath temp = *i + "scsi_host";
        if (temp) {
            AHCI_Phy *pPhy = new AHCI_Phy(CanonicalPath(*i), number++, this);
            attachPhy(pPhy);
            pPhy->discover();
        }
    }
}

/* */
void AHCI::getAddress(SSI_Address &address) const
{
    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = SSI_FALSE;
    address.sasAddress = 0ULL;
}

RaidInfo *AHCI::findRaidInfo()
{
    struct orom_info *pInfo = efi_get(getControllerType());
    if (pInfo == 0)
        pInfo = orom_get(m_PciDeviceId);
    if (pInfo != 0) {
        m_pRaidInfo = new AHCI_RaidInfo(this,pInfo);
        return m_pRaidInfo;
    }
    return 0;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
