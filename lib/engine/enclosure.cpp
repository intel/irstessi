
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
#include <stddef.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "routing_device.h"
#include "enclosure.h"
#include "session.h"
#include "utils.h"
#include "storage_device.h"
#include "end_device.h"

/* */
Enclosure::Enclosure(const String &path)
    : StorageObject(path),
      m_LogicalId("")
{
    CanonicalPath temp;
    temp = m_Path + "/generic";
    m_SgName = temp.reverse_after("/");

    try {
        SysfsAttr attr = m_Path + "/vendor";
        attr >> m_VendorId;
    } catch (...) {
    }
    try {
        SysfsAttr attr = m_Path + "/model";
        attr >> m_ProductId;
    } catch (...) {
    }
    try {
        SysfsAttr attr = m_Path + "/rev";
        attr >> m_Rev;
    } catch (...) {
    }
    Directory dir = m_Path + "/enclosure";
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        try {
            SysfsAttr attr =  *(*i) + "components";
            attr >> m_SlotCount;
        } catch (...) {
        }
    }

    String sbuffer;
    if (shell_cap("sg_ses -p 0x1 /dev/" + m_SgName, sbuffer) == 0) {
        m_LogicalId = sbuffer.between("logical identifier (hex): ", "\n");
        m_LogicalId.trim();
        m_SubenclosureCount = sbuffer.between("number of secondary subenclosures:", "\n");
    }
    shell_cap("sg_ses -p 0xa /dev/" + m_SgName, sbuffer);
    if (sbuffer)
        __get_slot_info(sbuffer);

}

/* */
Enclosure::~Enclosure()
{
    for (Iterator<Slot *> i = m_Slots; *i != 0; ++i)
        delete *i;
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
    pInfo->enclosureKey = getId();
    m_VendorId.get(pInfo->vendorInfo, sizeof(pInfo->vendorInfo));
    m_Rev.get(pInfo->productRev, sizeof(pInfo->productRev));
    m_ProductId.get(pInfo->productId, sizeof(pInfo->productId));
    m_LogicalId.get(pInfo->logicalId, sizeof(pInfo->logicalId));
    pInfo->processorCount = 0;
    pInfo->subenclosureCount = m_SubenclosureCount;
    pInfo->elementCount = m_Slots;
    pInfo->numberOfSlots = m_SlotCount;
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
String Enclosure::getKey() const
{
    return m_LogicalId;
}

/* */
void Enclosure::attachEndDevice(EndDevice *pEndDevice)
{
    m_EndDevices.add(pEndDevice);
}

/* */
void Enclosure::attachEndDevices(Container<EndDevice> &EndDevices)
{
    for (Iterator<EndDevice *> i = EndDevices; *i != 0; i++) {
        (*i)->setEnclosure(this);
        attachEndDevice(*i);
    }
}

/* */
void Enclosure::attachRoutingDevice(RoutingDevice *pRoutingDevice)
{
    m_RoutingDevices.add(pRoutingDevice);
}

/* */
void Enclosure::acquireId(Session *pSession)
{
    Container<EndDevice> container;
    for(Iterator<RoutingDevice *> i = m_RoutingDevices; *i != 0; i++) {
        (*i)->getEndDevices(container, false);
        attachEndDevices(container);
    }
    pSession->addEnclosure(this);
}

/* */
unsigned int Enclosure::getSlotNumber(unsigned long long sasAddress) const
{
    for (Iterator<Slot *> i = m_Slots; *i != 0; ++i)
        if ((*i)->sasAddress == sasAddress)
            return (*i)->slotNumber;
    return -1U;
}

/* */
void Enclosure::getSlotAddress(SSI_Address &address, unsigned int number)
{
    (void)address;
    (void)number;
}

/* */
void Enclosure::__get_slot_info(String &buffer)
{
    String info, right;
    unsigned int offset;
    right = buffer.after("element index");
    while (right) {
        try {
            /* find start of next info block */
            offset = right.find("element index");
        }
        catch (...) {
            offset = 0;
        }
        info = offset?right.left(offset):right;
        /* get info */
        String tmp = info.between("bay number: ", "\n");
        if (tmp) {
            Slot *pSlot = new Slot;
            pSlot->slotNumber = tmp;
            tmp = info.reverse_after("SAS address:");
            tmp = tmp.left("\n");
            pSlot->sasAddress = tmp;
            if (pSlot->sasAddress != 0)
                m_Slots.add(pSlot);
            else
                delete pSlot;
        }
        right = offset?right.get(offset+13):"";
    }
}

/* */
bool Enclosure::attachedTo(StorageObject *pObject) const
{
    for(Iterator<RoutingDevice *> i = m_RoutingDevices; *i != 0; ++i)
        if ((*i)->getParent() == pObject)
            return true;
    return false;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
