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
#include <sys/stat.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "cache.h"
#include "controller.h"
#include "phy.h"
#include "session.h"
#include "port.h"
#include "ahci.h"
#include "storage_device.h"
#include "end_device.h"
#include "block_device.h"
#include "multimedia_device.h"
#include "stream_device.h"
#include "routing_device.h"
#include "ahci_phy.h"
#include "ahci_disk.h"
#include "ahci_cdrom.h"
#include "ahci_tape.h"
#include "ahci_multiplier.h"

/* */
AHCI_Phy::AHCI_Phy(AHCI *pParent, const String &path, unsigned int number)
    : Phy(pParent, "", number)
{
    m_Path = CanonicalPath(path + "/scsi_host" + path.right("/host"));
    Directory dir(path, "target");
    if (dir > 0) {
        m_pPort = new Port(pParent, path);
        m_pPort->attachPhy(this);
        if (dir.count() == 1) {
            EndDevice *pEndDevice = __attachEndDevice(dir);
            if (pEndDevice != 0) {
                m_pPort->attachPort(pEndDevice->getPort());
            }
        } else {
            AHCI_Multiplier *pMultiplier = new AHCI_Multiplier(path, dir);
            m_pPort->attachPort(pMultiplier->getPort());
        }
        m_pParent->attachPort(m_pPort);
    }
}

/* */
SSI_Status AHCI_Phy::locate(bool mode) const
{
#if 0 /* APW */
    SysfsAttr em_message(m_Path + "/em_message");
    if (mode) {
        em_message << "1";
    } else {
        em_message << "0";
    }
#else /* APW */
    (void)mode;
#endif /* APW */
    return SSI_StatusOk;
}

/* */
EndDevice * AHCI_Phy::__attachEndDevice(const Path &path)
{
    CanonicalPath temp = path + "driver";
    EndDevice *pEndDevice = 0;
    if (temp = "/sys/bus/scsi/drivers/sd") {
        pEndDevice = new AHCI_Disk(path);
    } else
    if (temp = "/sys/bus/scsi/drivers/sr") {
        pEndDevice = new AHCI_CDROM(path);
    } else
    if (temp = "/sys/bus/scsi/drivers/st") {
        pEndDevice = new AHCI_Tape(path);
    }
    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
