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
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/stat.h>

#include <ssi.h>
#include <orom/orom.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "raid_info.h"
#include "cache.h"
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
    Directory dir(path, "host");
    unsigned int number = 0;
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        CanonicalPath temp = *(*i) + "scsi_host";
        if (temp) {
            attachPhy(new AHCI_Phy(this, CanonicalPath(*(*i)), number++));
        }
    }
    __internal_initialize();
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

/* */
void AHCI::__internal_initialize()
{
    SysfsAttr attr;
    struct PCIHeader pciInfo;

    /* TODO: read the name of controller from PCI bar */
    m_Name = "AHCI at " + m_Path.reverse_right("0000:");

    try {
        attr = m_Path + "/driver/module/version";
        attr >> m_DriverVersion;
    } catch (...) {
        /* TODO: log that version of the driver cannot be determined. */
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

    struct orom_info *pInfo = orom_get(m_PciDeviceId);
    if (pInfo != 0) {
        m_PrebootMgrVersion =
            String(pInfo->prod_ver.major) + String(".") +
            String(pInfo->prod_ver.minor) + String(".") +
            String(pInfo->prod_ver.hotfix) + String(".") +
            String(pInfo->prod_ver.build);
        m_twoTbVolumePrebootSupported = pInfo->a_2tb_vol;
        m_twoTbDiskPrebootSupported = pInfo->a_2tb_disk;
        m_ESATASpanning = pInfo->c_esata;
        m_NVSRAMSupported = pInfo->a_nvm;
        m_HWXORSupported = pInfo->f_hardware_xor;
        m_PhyLocate = pInfo->f_led_locate;
        m_DiskUnlock = pInfo->c_hdd_passwd;
        m_PatrolReadSupport = pInfo->f_read_patrol;

        m_pRaidInfo = new AHCI_RaidInfo(this, pInfo->dpa, pInfo->tds,
            pInfo->vpa, pInfo->vphba, pInfo->chk);
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
