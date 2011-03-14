/*
 * IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING.
 * BY LOADING OR USING THIS SOFTWARE, YOU AGREE TO THE TERMS OF THIS INTEL
 * SOFTWARE LICENSE AGREEMENT.  IF YOU DO NOT WISH TO SO AGREE, DO NOT COPY,
 * INSTALL OR USE THIS SOFTWARE.
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
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
class RaidInfo;

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
    virtual void getEndDevices(Container &, bool) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getRoutingDevices(Container &, bool) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getPorts(Container &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getVolumes(Container &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getArrays(Container &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getControllers(Container &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getPhys(Container &) const {
        throw E_INVALID_SCOPE;
    }
    virtual void getEnclosures(Container &, bool) const {
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
    void getEndDevices(Container &, bool) const;
    void getRoutingDevices(Container &, bool) const;
    void getPorts(Container &) const;
    void getVolumes(Container &) const;
    void getArrays(Container &) const;
    void getControllers(Container &) const;
    void getPhys(Container &) const;
    void getEnclosures(Container &, bool) const;

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
    StorageObject(StorageObject *pParent)
        : m_pParent(pParent) {
    }
    StorageObject(StorageObject *pParent, const String &path)
        : m_pParent(pParent), m_Path(path) {
    }

protected:
    StorageObject *m_pParent;
    String m_Path;

public:
    virtual void attachEndDevice(Object *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachRoutingDevice(Object *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPort(Object *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachVolume(Object *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachArray(Object *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachPhy(Object *) {
        throw E_INVALID_OPERATION;
    }
    virtual void attachEnclosure(Object *) {
        throw E_INVALID_OPERATION;
    }
    StorageObject * getParent() const {
        return m_pParent;
    }
    virtual void setParent(StorageObject *pParent) {
        m_pParent = pParent;
    }
    virtual RaidInfo * getRaidInfo() const {
        return 0;
    }
    virtual void getAddress(SSI_Address &) const {
        throw E_INVALID_OPERATION;
    }

public:
    virtual void acquireId(Session *pSession) = 0;
};

#endif /* __OBJECT_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
