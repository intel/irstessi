/*
Copyright (c) 2011 - 2016, Intel Corporation
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

#include "isci_expander_phy.h"
#include "isci_expander_port.h"
#include "routing_device.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
ISCI_Expander_Phy::ISCI_Expander_Phy(const String &path, unsigned int number, const Parent& pParent)
    : Phy(path, number, pParent)
{
}

/* */
void ISCI_Expander_Phy::discover()
{
    CanonicalPath portPath(m_Path + "/port");
    if (portPath) {
        shared_ptr<Port> port;
        if (Parent parent = m_pParent.lock()) {
            port = parent->getPortByPath(portPath);
            m_pPort = port;
            if (!port) {
                port = __internal_create_port(portPath);
                port->setParent(parent);
                m_pPort = port;
            }
        }

        if (port) {
            port->attachPhy(shared_from_this());
        }
    }
}

/* */
shared_ptr<Port> ISCI_Expander_Phy::__internal_create_port(const String &portPath)
{
    Directory dir(portPath);
    shared_ptr<Port> pPort;
    shared_ptr<RoutingDevice> pRtDevice = dynamic_pointer_cast<RoutingDevice>(m_pParent.lock());

    dir.setFilter("host");
    if (dir > 0) {
        /* host" */
        pPort = shared_ptr<Port>(new ISCI_Expander_SubtractivePort(portPath));
        if (pRtDevice) {
            pRtDevice->setSubtractivePort(pPort);
        }
        return pPort;
    }

    dir.setFilter("expander");
    if (dir > 0) {
        if (portPath.compare(CanonicalPath(**dir.dirs().begin())) > 0) {
            /* host of expander behind expander */
            pPort = shared_ptr<Port>(new ISCI_Expander_SubtractivePort(portPath));
            if (pRtDevice) {
                pRtDevice->setSubtractivePort(pPort);
            }
        } else {
            /* expander in expander */
            pPort = shared_ptr<Port>(new ISCI_Expander_Port(portPath));
            if (Parent parent = m_pParent.lock()) {
                parent->attachPort(pPort);
            }
        }

        return pPort;
    }

    pPort = shared_ptr<Port>(new ISCI_Expander_Port(portPath));
    if (Parent parent = m_pParent.lock()) {
        parent->attachPort(pPort);
    }

    return pPort;
}


/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab : */
