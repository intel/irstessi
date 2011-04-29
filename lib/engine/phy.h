
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

#ifndef __PHY_H__INCLUDED__
#define __PHY_H__INCLUDED__

// Forward declaration
class Port;

/* */
class Phy : public StorageObject {
public:
    Phy(const String &path, unsigned int number);
    virtual ~Phy();

    // Object

public:
    ObjectType getType() const {
        return ObjectType_Phy;
    }
    bool equal(const Object *pObject) const;

    // ScopeObject

public:
    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypePhy;
    }

    // StorageObject

public:
    void acquireId(Session *pSession);

    void attachPhy(Phy *pPhy);
    void attachPort(Port *pPort);

    // Phy

public:
    void setNumber(unsigned int number) {
        m_Number = number;
    }
    unsigned int getNumber() const {
        return m_Number;
    }
    virtual SSI_Status locate(bool mode) const;
    SSI_Status getInfo(SSI_PhyInfo *pInfo) const;

protected:
    Phy *m_pRemotePhy;
    Port *m_pPort;
    unsigned int m_Number;
};

#endif /* __PHY_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
