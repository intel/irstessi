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

#include <unistd.h>
#include <asm/types.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "cache.h"
#include "controller.h"
#include "isci.h"
#include "raid_info.h"
#include "phy.h"
#include "session.h"
#include "enclosure.h"
#include "isci_phy.h"
#include "port.h"
#include "pci_header.h"

/* */
ISCI::ISCI(const String &path)
    : Controller(path)
{
    SysfsAttr attr;
    struct PCIHeader pciInfo;

    m_Name = "ISCI at " + m_Path.reverse_right("0000:");

    try {
        attr = m_Path + "/driver/module/version";
        attr >> m_DriverVersion;
    } catch (...) {
        m_DriverVersion = "Unknown";
    }
    try {
        attr = m_Path + "/config";
        attr.read(&pciInfo, sizeof(struct PCIHeader));
        m_PciVendorId = pciInfo.vendorId;
        m_PciDeviceId = pciInfo.deviceId;
        m_SubSystemId = pciInfo.subSystemId;
        m_HardwareRevisionId = pciInfo.revisionId;
        m_SubClassCode = pciInfo.subClassId;
        m_SubVendorId = pciInfo.subSystemVendorId;
    } catch (...) {
        /* TODO: log that PCI header cannot be read from sysfs. */
    }

    /* TODO: Read RAID info parameters from EFI variables if any */
}

/* */
void ISCI::discover()
{
    Directory dir(m_Path, "host");
    SysfsAttr attr;
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        unsigned int number = 0;
        try {
            attr = *(*i) + "scsi_host" + (*i)->reverse_after("/") + "isci_id";
            attr >> number;
        } catch (Exception) {
            /* TODO: report read failure of attribtue. */
        }
        Directory phys(*(*i), "phy");
        number *= 4;
        for (Iterator<Directory *> j = phys; *j != 0; ++j, ++number) {
            Phy *pPhy = new ISCI_Phy(*(*j), number);
            attachPhy(pPhy);
            pPhy->setParent(this);
        }
    }
    for (Iterator<Phy *> i = m_Phys; *i != 0; ++i) {
        (*i)->discover();
    }
    for (Iterator<Port *> i = m_Ports; *i != 0; ++i) {
        (*i)->discover();
    }
}

/* */
Port * ISCI::getPortByPath(const String &path) const
{
    for (Iterator<Port *> i = m_Ports; *i != 0; ++i) {
        if ((*i)->getPath() == path) {
            return (*i);
        }
    }
    return 0;
}

/* */
void ISCI::getAddress(SSI_Address &address) const
{
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
