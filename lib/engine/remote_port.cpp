
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
#include "object.h"
#include "phy.h"
#include "storage_device.h"
#include "end_device.h"
#include "routing_device.h"
#include "port.h"
#include "remote_port.h"
#include "session.h"

/* */
RemotePort::RemotePort(const String &path)
    : Port(path)
{
}

/* */
void RemotePort::attachPort(Port *pPort)
{
    if (pPort == this)
        throw E_INVALID_OBJECT;

    m_pRemotePort = pPort;
    if (RoutingDevice *tmp = dynamic_cast<RoutingDevice *>(m_pParent))
        m_pRemotePort->attachRoutingDevice(tmp);
    else if (EndDevice *tmp = dynamic_cast<EndDevice *>(m_pParent))
        m_pRemotePort->attachEndDevice(tmp);
}

/* */
RaidInfo * RemotePort::getRaidInfo() const
{
    return m_pRemotePort->getRaidInfo();
}

/* */
void RemotePort::attachArray(Array *pArray)
{
    m_pRemotePort->attachArray(pArray);
}

/* */
void RemotePort::attachVolume(Volume *pVolume)
{
    m_pRemotePort->attachVolume(pVolume);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
