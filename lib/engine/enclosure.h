/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __ENCLOSURE_H__INCLUDED__
#define __ENCLOSURE_H__INCLUDED__

#include "storage_object.h"
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

typedef struct __Slot {
    unsigned int slotNumber;
    unsigned long long sasAddress;
} Slot;

/* */
class Enclosure : public StorageObject, public boost::enable_shared_from_this<Enclosure> {
public:
    Enclosure(const String &path);

public:
    virtual void addToSession(const boost::shared_ptr<Session>& pSession);
    SSI_Status getInfo(SSI_EnclosureInfo *pInfo) const;
    virtual void getEndDevices(Container<EndDevice> &, bool all) const;
    virtual void getRoutingDevices(Container<RoutingDevice> &, bool all) const;
    String getLogicalId() const;
    virtual bool operator ==(const Object &object) const;
    virtual String getId() const;
    virtual void attachEndDevice(const boost::shared_ptr<EndDevice>& endDevice);
    virtual void attachEndDevices(Container<EndDevice> &EndDevices);
    virtual void attachRoutingDevice(const boost::shared_ptr<RoutingDevice>& routingDevice);
    unsigned int getSlotNumber(unsigned long long sasAddress) const;
    void getSlotAddress(SSI_Address &address, unsigned int number);

    virtual bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeEnclosure;
    }
    bool attachedTo(const boost::shared_ptr<StorageObject>& pObject) const;

protected:
    Container<EndDevice> m_EndDevices;
    Container<RoutingDevice> m_RoutingDevices;
    String m_SgName;
    String m_LogicalId;
    String m_VendorId;
    String m_ProductId;
    String m_Rev;
    Container<Slot> m_Slots;
    unsigned int m_SlotCount;
    unsigned int m_SubenclosureCount;

private:
    void __get_slot_info(String &buffer);
};

#endif /* __ENCLOSURE_H__INCLUDED__ */
