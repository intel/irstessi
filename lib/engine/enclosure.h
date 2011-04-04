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
};

#endif /* __ENCLOSURE_H__INCLUDED__ */
