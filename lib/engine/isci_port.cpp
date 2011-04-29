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
#include "storage_device.h"
#include "end_device.h"
#include "nondisk_device.h"
#include "block_device.h"
#include "multimedia_device.h"
#include "stream_device.h"
#include "routing_device.h"
#include "isci.h"
#include "isci_port.h"
#include "isci_phy.h"
#include "isci_expander.h"
#include "isci_disk.h"
#include "isci_cdrom.h"
#include "isci_tape.h"

/* */
ISCI_Port::ISCI_Port(const String &path)
    : Port(path)
{
}

/* */
void ISCI_Port::discover()
{
    Iterator<Directory *> i;
    Directory dir(m_Path, "expander");
    if (*(i = dir) != 0) {
        ISCI_Expander *pExpander = new ISCI_Expander(*(*i));
        pExpander->discover();
        attachPort(pExpander->getSubtractivePort());
        return;
    }
    dir.setFilter("end_device");
    if (*(i = dir) != 0) {
        Directory target(*(*i), "target");
        EndDevice *pEndDevice = __internal_create_end_device(target);
        if (pEndDevice != 0) {
            attachPort(pEndDevice->getPort());
        }
        return;
    }
    dir.setFilter("host");
    if (*(i = dir) == 0) {
        throw E_INVALID_OBJECT;
    }
}

/* */
EndDevice * ISCI_Port::__internal_create_end_device(Iterator<Directory *> i)
{
    EndDevice *pEndDevice = 0;
    if (*i != 0)
    for (Iterator<Directory *> j = *(*i); *j != 0; ++j) {
        CanonicalPath temp = *(*j) + "driver";
        if (temp == "/sys/bus/scsi/drivers/sd") {
            pEndDevice = new ISCI_Disk(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/sr") {
            pEndDevice = new ISCI_CDROM(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/st") {
            pEndDevice = new ISCI_Tape(*(*j));
            break;
        }
    }
    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
