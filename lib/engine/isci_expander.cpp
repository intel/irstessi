
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
        Phy *pPhy = new ISCI_Expander_Phy(*(*i), number++, this);
        attachPhy(pPhy);
    }
    for (Iterator<Phy *> i = m_Phys; *i != 0; ++i) {
        (*i)->discover();
    }
    for(Iterator<Port *> i = m_Ports; *i != 0; ++i) {
        (*i)->discover();
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
