
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if __GNUC_PREREQ(3, 4)
#pragma once
#endif /* __GNUC_PREREQ */

#ifndef __ENCLOSURE_H__INCLUDED__
#define __ENCLOSURE_H__INCLUDED__

/* */
class Enclosure : public StorageObject {
public:
    Enclosure(const String &path);
    ~Enclosure();

public:
    void acquireId(Session *pSession);
    SSI_Status getInfo(SSI_EnclosureInfo *pInfo) const;
    void getEndDevices(Container<EndDevice> &, bool all) const;
    void getRoutingDevices(Container<RoutingDevice> &, bool all) const;
    String getLogicalId() const;
    bool equal(const Object *pObject) const;
    void attachEndDevice(EndDevice *);
    void attachRoutingDevice(RoutingDevice *);
    void getSlotAddress(SSI_Address &address, unsigned int number);

    ObjectType getType() const {
        return ObjectType_Enclosure;
    }
    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeEnclosure;
    }

protected:
    Container<EndDevice> m_EndDevices;
    Container<RoutingDevice> m_RoutingDevices;
    String m_LogicalId;
};

#endif /* __ENCLOSURE_H__INCLUDED__ */
