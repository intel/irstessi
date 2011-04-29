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
#include "log/log.h"
/* */
ISCI_Expander::ISCI_Expander(const String &path)
    : RoutingDevice(path)
{
	Directory dir(m_Path + "/sas_device", "expander");
	SysfsAttr attr;
	for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        try {
            attr = *(*i) + "sas_address";
            attr >> m_SasAddress;
        } catch (...) {
            /* TODO: report read failure of attribtue. */
        }
	}
	dlog(" sas adress expandera %s\n%s\n", (const char *) path, (const char *) m_SasAddress);
}

/* */
Port * ISCI_Expander::getPortByPath(const String &path) const
{
    for (Iterator<Port *> i = m_Ports; *i != 0; ++i) {
        if ((*i)->getPath() == path) {
            return (*i);
        }
    }
    if (m_pSubtractivePort) {
        if (m_pSubtractivePort->getPath() == path) {
            return m_pSubtractivePort;
        }
    }
    return 0;
}

/* */
void ISCI_Expander::discover()
{
    Directory dir(m_Path, "phy");
    unsigned int number = 0;
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        Phy *pPhy = new ISCI_Expander_Phy(*(*i), number++);
        attachPhy(pPhy);
        pPhy->setParent(this);
    }
    for (Iterator<Phy *> i = m_Phys; *i != 0; ++i) {
        (*i)->discover();
    }
    for(Iterator<Port *> i = m_Ports; *i != 0; ++i) {
        (*i)->discover();
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
