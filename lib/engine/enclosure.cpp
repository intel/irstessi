
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
#include "enclosure.h"
#include "session.h"
#include "utils.h"

/* */
Enclosure::Enclosure(const String &path)
    : StorageObject(path),
      m_LogicalId("")
{
    CanonicalPath temp;
    temp = m_Path + "/generic";
    m_SgName = temp.reverse_after("/");
    String sbuffer;
    if (shell_cap("sg_ses -p 0x1 /dev/" + m_SgName, sbuffer) == 0) {
        m_LogicalId = sbuffer.between("logical identifier (hex): ", "\n");
        m_LogicalId.trim();
        m_VendorId = sbuffer.between("vendor: ", "product");
        m_VendorId.trim();
    }
}

/* */
Enclosure::~Enclosure()
{
}

/* */
void Enclosure::getEndDevices(Container<EndDevice> &container, bool) const
{
    container = m_EndDevices;
}

/* */
SSI_Status Enclosure::getInfo(SSI_EnclosureInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->enclosureHandle = getId();
    return SSI_StatusOk;
}

/* */
void Enclosure::getRoutingDevices(Container<RoutingDevice> &container, bool) const
{
    container = m_RoutingDevices;
}

/* */
String Enclosure::getLogicalId() const
{
    return m_LogicalId;
}

/* */
bool Enclosure::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
           m_LogicalId == dynamic_cast<const Enclosure*>(pObject)->getLogicalId();
}

/* */
void Enclosure::attachEndDevice(EndDevice *pEndDevice)
{
    m_EndDevices.add(pEndDevice);
}

/* */
void Enclosure::attachRoutingDevice(RoutingDevice *pRoutingDevice)
{
    m_RoutingDevices.add(pRoutingDevice);
}

/* */
void Enclosure::acquireId(Session *pSession)
{
    pSession->addEnclosure(this);
}

/* */
void Enclosure::getSlotAddress(SSI_Address &address, unsigned int number)
{
    (void)address;
    (void)number;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
