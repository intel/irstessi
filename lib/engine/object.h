
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

#ifndef __OBJECT_H__INCLUDED__
#define __OBJECT_H__INCLUDED__

/* */
enum ObjectType {
    ObjectType_First = 0,
    ObjectType_Session = ObjectType_First,
    ObjectType_Event,
    ObjectType_EndDevice,
    ObjectType_Array,
    ObjectType_Enclosure,
    ObjectType_Phy,
    ObjectType_Volume,
    ObjectType_Port,
    ObjectType_RoutingDevice,
    ObjectType_RaidInfo,
    ObjectType_Controller,
    ObjectType_Last,
    ObjectType_Unknown = -1
};

/* */
inline ObjectType getTypeOfId(unsigned int id) {
    return static_cast<ObjectType>(id >> 28);
}

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
class Object {
public:
    Object(const Object &object)
        : m_Id(object.m_Id) {
    }
    Object()
        : m_Id(0) {
    }
    virtual ~Object() {
    }

public:
    virtual ObjectType getType() const {
        return ObjectType_Unknown;
    }
    unsigned int getId() const {
        return m_Id;
    }
    operator unsigned int () const {
        return getId();
    }
    operator ObjectType () const {
        return getType();
    }
    bool operator == (const Object *pObject) const {
        try {
            return equal(pObject);
        } catch (...) {
            return false;
        }
    }
    Object & operator = (unsigned int id) {
        setId(id); return *this;
    }
    void setId(unsigned int id) {
        m_Id = id;
    }
    virtual bool equal(const Object *pObject) const {
        return pObject && getType() == pObject->getType();
    }
    virtual String getKey() const {
        return "";
    }

protected:
    unsigned int m_Id;
};

/* */
inline bool operator == (const Object &object, unsigned int id) {
    return object.getId() == id;
}
inline bool operator == (unsigned int id, const Object &object) {
    return id == object.getId();
}
inline bool operator != (const Object &object, unsigned int id) {
    return object.getId() != id;
}
inline bool operator != (unsigned int id, const Object &object) {
    return id != object.getId();
}

/* */
class ScopeObject : public Object {
public:
    ScopeObject(const ScopeObject &scopeObject)
        : Object(scopeObject) {
    }
    ScopeObject() {
    }
    virtual ~ScopeObject() {
    }

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

/* */
inline bool operator != (const ScopeObject &object, SSI_ScopeType scopeType) {
    return object.scopeTypeMatches(scopeType) == false;
}

/* */
inline bool operator != (SSI_ScopeType scopeType, const ScopeObject &object) {
    return object.scopeTypeMatches(scopeType);
}

/* */
inline bool operator == (const ScopeObject &object, SSI_ScopeType scopeType) {
    return object.scopeTypeMatches(scopeType);
}

/* */
inline bool operator == (SSI_ScopeType scopeType, const ScopeObject &object) {
    return object.scopeTypeMatches(scopeType);
}

/**
 */
class NoneScopeObject : public ScopeObject {
public:
    NoneScopeObject(Session *pSession)
        : m_pSession(pSession) {
    }

protected:
    Session *m_pSession;

public:
    void getEndDevices(Container<EndDevice> &, bool) const;
    void getRoutingDevices(Container<RoutingDevice> &, bool) const;
    void getPorts(Container<Port> &) const;
    void getVolumes(Container<Volume> &) const;
    void getArrays(Container<Array> &) const;
    void getControllers(Container<Controller> &) const;
    void getPhys(Container<Phy> &) const;
    void getEnclosures(Container<Enclosure> &, bool) const;

    bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeNone;
    }
};

/**
 */
class StorageObject : public ScopeObject {
public:
    virtual ~StorageObject() {
    }
    StorageObject(StorageObject *pParent = NULL)
        : m_pParent(pParent) {
    }
    StorageObject(const String &path, StorageObject *pParent = NULL)
        : m_pParent(pParent), m_Path(path) {
    }

protected:
    StorageObject *m_pParent;
    String m_Path;

public:
    String getPath() const {
        return m_Path;
    }
    void setPath(const String &path) {
        m_Path = path;
    }

public:
    virtual void attachEndDevice(EndDevice *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachRoutingDevice(RoutingDevice *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPort(Port *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachVolume(Volume *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachArray(Array *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPhy(Phy *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachEnclosure(Enclosure *) {
        throw E_INVALID_OPERATION;
    }
    StorageObject * getParent() const {
        return m_pParent;
    }
    void setParent(StorageObject *pParent) {
        if (pParent != this) {
            m_pParent = pParent;
        }
    }
    virtual Controller * getController() const {
        return NULL;
    }
    virtual RaidInfo * getRaidInfo() const {
        return NULL;
    }
    virtual void getAddress(SSI_Address &) const {
        throw E_INVALID_OPERATION;
    }
    virtual void setAddress(SSI_Address &) {
        throw E_INVALID_OPERATION;
    }

    virtual SSI_Status locate(bool mode) {
        return SSI_StatusNotSupported;
    }

public:
    virtual Port * getPort() const {
        throw E_INVALID_OPERATION;
    }
    virtual Port * getPortByPath(const String &s) const {
        throw E_INVALID_OPERATION;
    }
    virtual void discover() {
        throw E_INVALID_OPERATION;
    }
    virtual void acquireId(Session *pSession) = 0;
};

#endif /* __OBJECT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
