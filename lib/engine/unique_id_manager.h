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

#ifndef __UNIQUE_ID_MANAGER_H__INCLUDED__
#define __UNIQUE_ID_MANAGER_H__INCLUDED__

/* */
class Id {
public:
    Id(unsigned int id)
        : m_Id(id) {
    }
    ~Id() {
    }
    unsigned int getId() const {
        return m_Id;
    }
    void add(Object * pObject) {
        if (pObject == 0) {
            throw E_NULL_POINTER;
        }
        m_Objects.add(pObject);
    }
    void remove(Object *pObject) {
        if (pObject == 0) {
            throw E_NULL_POINTER;
        }
        m_Objects.remove(pObject);
    }
    unsigned int count() const {
        return m_Objects.count();
    }

public:
    bool operator == (const Object *pObject) const;
    bool operator != (const Object *pObject) const;

private:
    unsigned int m_Id;
    Cache m_Objects;

    friend class UniqueIdManager;
};

/* */
class IdCache : public List<Id *> {
public:
    IdCache();
    ~IdCache();

    void remove(Object *pObject);
    void add(Object *pObject);

private:
    unsigned int __findId() const;
};

/* */
class UniqueIdManager {
public:
    UniqueIdManager();
    ~UniqueIdManager();

private:
    IdCache m_Sessions;
    IdCache m_Events;
    IdCache m_EndDevices;
    IdCache m_Arrays;
    IdCache m_Enclosures;
    IdCache m_Phys;
    IdCache m_Volumes;
    IdCache m_Ports;
    IdCache m_RoutingDevices;
    IdCache m_RaidInfo;
    IdCache m_Controllers;

public:
    unsigned int acquireId(Object *);
    void releaseId(Object *);
};

#endif /* __UNIQUE_ID_MANAGER_H__INCLUDED__ */
