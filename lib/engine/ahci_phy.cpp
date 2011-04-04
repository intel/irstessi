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
#include "raid_info.h"
#include "cache.h"
#include "controller.h"
#include "phy.h"
#include "session.h"
#include "port.h"
#include "ahci.h"
#include "storage_device.h"
#include "end_device.h"
#include "nondisk_device.h"
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
#define EM_MSG_WAIT     1500

/* */
AHCI_Phy::AHCI_Phy(const String &path, unsigned int number)
    : Phy(path, number), m_PhyPath(CanonicalPath(path + "/scsi_host" + path.reverse_right("/host")))
{
}

/* */
void AHCI_Phy::discover()
{
    Directory dir(m_Path, "target");
    if (dir > 0) {
        m_pPort = new Port(m_Path);
        m_pPort->setParent(m_pParent);
        m_pPort->attachPhy(this);

        if (dir.count() == 1) {
            EndDevice *pEndDevice = __internal_attach_end_device(dir);
            if (pEndDevice != 0) {
                m_pPort->attachPort(pEndDevice->getPort());
            }
        } else {
            AHCI_Multiplier *pMultiplier = new AHCI_Multiplier(m_Path, dir);
            m_pPort->attachPort(pMultiplier->getSubtractivePort());
        }
        if (m_pParent != 0) {
            m_pParent->attachPort(m_pPort);
        }
    }
}

/* */
SSI_Status AHCI_Phy::locate(bool mode) const
{
    SysfsAttr em_message(m_Path + "/em_message");
    try {
        usleep(EM_MSG_WAIT);
        if (mode) {
            em_message << 0x00080000;
        } else {
            em_message << 0x00080000;
        }
        return SSI_StatusOk;
    } catch (...) {
        return SSI_StatusFailed;
    }
}

/* */
EndDevice * AHCI_Phy::__internal_attach_end_device(Iterator<Directory *> i)
{
    EndDevice *pEndDevice = 0;
    for (Iterator<Directory *> j = *(*i); *j != 0; ++j) {
        CanonicalPath temp = *(*j) + "driver";
        if (temp == "/sys/bus/scsi/drivers/sd") {
            pEndDevice = new AHCI_Disk(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/sr") {
            pEndDevice = new AHCI_CDROM(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/st") {
            pEndDevice = new AHCI_Tape(*(*j));
            break;
        }
    }
    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
