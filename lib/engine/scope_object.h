/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SCOPE_OBJECT_H__INCLUDED__
#define __SCOPE_OBJECT_H__INCLUDED__

#include "object.h"
#include "container.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

// Forward declaration
class Session;
class RoutingDevice;
class EndDevice;
class Port;
class RaidInfo;
class Volume;
class Controller;
class Phy;
class Enclosure;
class Array;

/* */
class ScopeObject : public Object {
public:
    virtual void getEndDevices(Container<EndDevice> &, bool) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getRoutingDevices(Container<RoutingDevice> &, bool) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getPorts(Container<Port> &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getVolumes(Container<Volume> &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getArrays(Container<Array> &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getControllers(Container<Controller> &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getPhys(Container<Phy> &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getEnclosures(Container<Enclosure> &, bool) const {
        throw E_INVALID_SCOPE;
    }
    virtual bool scopeTypeMatches(SSI_ScopeType) const {
        return false;
    }
};

#endif /* __SCOPE_OBJECT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
