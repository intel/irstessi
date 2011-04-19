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
#include "phy.h"
#include "routing_device.h"
#include "port.h"
#include "remote_port.h"
#include "isci_phy.h"
#include "isci_port.h"
#include "isci_expander_port.h"
#include "isci_expander_phy.h"

/* */
ISCI_Expander_Phy::ISCI_Expander_Phy(const String &path, unsigned int number)
    : Phy(path, number)
{
}

/* */
void ISCI_Expander_Phy::discover()
{
    CanonicalPath portPath(m_Path + "/port");
    if (portPath) {
        m_pPort = m_pParent->getPortByPath(portPath);
        if (m_pPort == 0) {
            m_pPort = __internal_create_port(portPath);
            m_pPort->setParent(m_pParent);
        }
        m_pPort->attachPhy(this);
    }
}

/* */
Port * ISCI_Expander_Phy::__internal_create_port(const String &portPath)
{
    Directory dir(portPath);
    Port *pPort = 0;
    RoutingDevice *pRtDevice = dynamic_cast<RoutingDevice *>(m_pParent);
    dir.setFilter("host");
    if (dir > 0) {
        pPort = new ISCI_Expander_SubtractivePort(portPath);
        if (pRtDevice) {
            pRtDevice->setSubtractivePort(pPort);
        }
    } else {
        dir.setFilter("expander");
        if (dir > 0) {
            Iterator<Directory *> i = dir;
            if (portPath.compare(CanonicalPath(*(*i))) > 0) {
                pPort = new ISCI_Expander_SubtractivePort(portPath);
                if (pRtDevice) {
                    pRtDevice->setSubtractivePort(pPort);
                }
            }
        } else {
            pPort = new ISCI_Expander_Port(portPath);
            m_pParent->attachPort(pPort);
        }
    }
    return pPort;
}

/* */
SSI_Status ISCI_Expander_Phy::locate(bool mode) const
{
    (void)mode;
    return SSI_StatusOk;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab : */
