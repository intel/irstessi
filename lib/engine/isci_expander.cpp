
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
#include <cstddef>

#include <ssi.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "storage_device.h"
#include "phy.h"
#include "port.h"
#include "end_device.h"
#include "nondisk_device.h"
#include "routing_device.h"
#include "session.h"
#include "block_device.h"
#include "multimedia_device.h"
#include "stream_device.h"
#include "remote_port.h"
#include "isci_phy.h"
#include "isci_expander_phy.h"
#include "isci_expander.h"
#include "utils.h"

#include "log/log.h"

/* */
ISCI_Expander::ISCI_Expander(const String &path)
    : RoutingDevice(path),
      m_SASAddress(0)
{
    Directory dir(m_Path + "/sas_device");
    SysfsAttr attr;
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        try {
            attr = *(*i) + "sas_address";
            attr >> m_SASAddress;
        } catch (...) {
            /* TODO: report read failure of attribtue. */
        }
    }
    dlog(" sas adress %s\n%llu\n", (const char *) path, m_SASAddress);
    dir = m_Path + "/sas_expander";
    dirs = dir.dirs();
    foreach (i, dirs) {
        try {
            attr = *(*i) + "product_id";
            attr >> m_ProductId;
        } catch (...) {
        }
        try {
            attr = *(*i) + "vendor_id";
            attr >> m_Vendor;
        } catch (...) {
        }
        try {
            attr = *(*i) + "product_rev";
            attr >> m_ProductRev;
        } catch (...) {
        }
        try {
            attr = *(*i) + "component_vendor_id";
            attr >> m_ComponentVendorId;
        } catch (...) {
        }
        try {
            attr = *(*i) + "component_id";
            attr >> m_ComponentId;
        } catch (...) {
        }
        try {
            attr = *(*i) + "component_revision_id";
            attr >> m_ComponentRev;
        } catch (...) {
        }
    }
}

/* */
Port * ISCI_Expander::getPortByPath(const String &path) const
{
    foreach (i, m_Ports) {
        if ((*i)->getPath() == path) {
            return (*i);
        }
    }
    if (m_pSubtractivePort) {
        if (m_pSubtractivePort->getPath() == path) {
            return m_pSubtractivePort;
        }
    }
    return NULL;
}

/* */
void ISCI_Expander::getAddress(SSI_Address &address) const
{
    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = m_SASAddress?SSI_TRUE:SSI_FALSE;
    address.sasAddress = m_SASAddress;
}

/* */
void ISCI_Expander::discover()
{
    Directory dir(m_Path, "phy");
    unsigned int number = 0;
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        Phy *pPhy = new ISCI_Expander_Phy(*(*i), number++, this);
        attachPhy(pPhy);
    }
    foreach (i, m_Phys)
        (*i)->discover();
    foreach (i, m_Ports)
        (*i)->discover();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
