/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __AHCI_MULTIPLIER_H__INCLUDED__
#define __AHCI_MULTIPLIER_H__INCLUDED__

#include "routing_device.h"
#include "phy.h"
#include "filesystem.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class AHCI_Multiplier : public RoutingDevice {
public:
    AHCI_Multiplier(const String &path, Directory &dir);
    ~AHCI_Multiplier() {
        if (m_pPhy) {
            delete m_pPhy;
        }
    }

    // ScopeObject

public:
    void getPhys(Container<Phy> &container) const;

    // StorageObject

public:
    void getAddress(SSI_Address &address) const;

    // RoutingDevice

public:
    unsigned int getNumberOfPhys() const {
        return RoutingDevice::getNumberOfPhys() + 1;
    }
    SSI_RoutingDeviceType getRoutingDeviceType() const {
        return SSI_RoutingDeviceTypeMultiplier;
    }

    void addToSession(Session *pSession);

    // AHCI_Multiplier

public:
    Phy * getPhy() const {
        return m_pPhy;
    }

protected:
    Phy *m_pPhy;

private:
    AHCI_Multiplier(const AHCI_Multiplier &multiplier);
    bool __internal_attach_end_device(const Path &path, unsigned int number);
    void operator = (const AHCI_Multiplier&) { }
};

#endif /* __AHCI_MULTIPLIER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
