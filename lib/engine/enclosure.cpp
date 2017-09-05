/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <cstddef>
#include <typeinfo>

#include "enclosure.h"
#include "filesystem.h"
#include "routing_device.h"
#include "controller.h"
#include "end_device.h"
#include "session.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
Enclosure::Enclosure(const String &path)
    : StorageObject(path),
      m_LogicalId(""),
      m_SlotCount(0),
      m_SubenclosureCount(0)
{
    CanonicalPath temp;
    File attr;
    temp = m_Path + "/generic";
    m_SgName = temp.reverse_after("/");

    try {
        attr = m_Path + "/vendor";
        attr >> m_VendorId;
    } catch (...) {
    }

    try {
        attr = m_Path + "/model";
        attr >> m_ProductId;
    } catch (...) {
    }

    try {
        attr = m_Path + "/rev";
        attr >> m_Rev;
    } catch (...) {
    }

    Directory dir = m_Path + "/enclosure";
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        try {
            attr =  *(*i) + "components";
            attr >> m_SlotCount;
        } catch (...) {
        }
    }

    String sbuffer;
    const String command1 = "sg_ses -p 0x1 /dev/" + m_SgName;
    if (shell_cap(command1, sbuffer) == 0) {
        m_LogicalId = sbuffer.between("logical identifier (hex): ", "\n");
        m_LogicalId.trim();
        m_SubenclosureCount = sbuffer.between("number of secondary subenclosures:", "\n");
    }
    const String command2 = "sg_ses -p 0xa /dev/" + m_SgName;
    shell_cap(command2, sbuffer);
    if (sbuffer) {
        __get_slot_info(sbuffer);
    }
}

/* */
String Enclosure::getId() const
{
    return "en:" + String(m_LogicalId);
}

/* */
void Enclosure::getEndDevices(Container<EndDevice> &container, bool) const
{
    container = m_EndDevices;
}

/* */
SSI_Status Enclosure::getInfo(SSI_EnclosureInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->enclosureHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    pInfo->enclosureKey = (getHandle() & 0x0fffffff);
    foreach (i, m_RoutingDevices) {
        if (shared_ptr<StorageObject> parent = (*i)->getParent().lock()) {
            if (parent && dynamic_pointer_cast<Controller>(parent)) {
                pInfo->enclosureKey |= 0x10000000;
                break;
            }
        }
    }
    m_VendorId.get(pInfo->vendorInfo, sizeof(pInfo->vendorInfo));
    m_Rev.get(pInfo->productRev, sizeof(pInfo->productRev));
    m_ProductId.get(pInfo->productId, sizeof(pInfo->productId));
    m_LogicalId.get(pInfo->logicalId, sizeof(pInfo->logicalId));
    pInfo->processorCount = 0;
    pInfo->subenclosureCount = m_SubenclosureCount;
    pInfo->elementCount = m_Slots.size();
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
bool Enclosure::operator ==(const Object &object) const
{
    return typeid(*this) == typeid(object) &&
           m_LogicalId == static_cast<const Enclosure *>(&object)->getLogicalId();
}

/* */
void Enclosure::attachEndDevice(const shared_ptr<EndDevice>& pEndDevice)
{
    m_EndDevices.add(pEndDevice);
}

/* */
void Enclosure::attachEndDevices(Container<EndDevice> &EndDevices)
{
    foreach (i, EndDevices) {
        (*i)->setEnclosure(shared_from_this());
        attachEndDevice(*i);
    }
}

/* */
void Enclosure::attachRoutingDevice(const shared_ptr<RoutingDevice>& pRoutingDevice)
{
    m_RoutingDevices.add(pRoutingDevice);
}

/* */
void Enclosure::addToSession(const shared_ptr<Session>& pSession)
{
    Container<EndDevice> container;
    foreach (i, m_RoutingDevices) {
        (*i)->getEndDevices(container, false);
        attachEndDevices(container);
    }
    pSession->addEnclosure(shared_from_this());
}

/* */
unsigned long long Enclosure::getSlotNumber(unsigned long long sasAddress) const
{
    foreach (i, m_Slots) {
        if ((*i)->sasAddress == sasAddress) {
            return (*i)->slotNumber;
        }
    }
    return -1ULL;
}

/* */
void Enclosure::getSlotAddress(SSI_Address &, unsigned int)
{
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
        info = offset ? right.left(offset) : right;
        /* get info */
        String tmp = info.between("bay number: ", "\n");
        if (tmp) {
            shared_ptr<Slot> pSlot = shared_ptr<Slot>(new Slot);
            pSlot->slotNumber = tmp;
            tmp = info.reverse_after("SAS address:");
            tmp = tmp.left("\n");
            pSlot->sasAddress = tmp;
            if (pSlot->sasAddress != 0) {
                m_Slots.add(pSlot);
            }
        }
        right = offset ? right.get(offset + 13) : "";
    }
}

/* */
bool Enclosure::attachedTo(const shared_ptr<StorageObject>& pObject) const
{
    foreach (i, m_RoutingDevices) {
        if ((*i)->getParent().lock() == pObject) {
            return true;
        }
    }

    return false;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
