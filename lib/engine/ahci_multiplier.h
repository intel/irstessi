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

#ifndef __AHCI_MULTIPLIER_H__INCLUDED__
#define __AHCI_MULTIPLIER_H__INCLUDED__

/* */
class AHCI_Multiplier : public RoutingDevice {
public:
    AHCI_Multiplier(const String &path, Directory &dir);

    // ScopeObject

public:
    void getPhys(Container &container) const;

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

    void acquireId(Session *pSession);

    // AHCI_Multiplier

public:
    Object * getPhy() const {
        return reinterpret_cast<Object *>(m_pPhy);
    }

protected:
    Phy *m_pPhy;

private:
    bool __internal_attach_end_device(const Path &path, unsigned int number);
};

#endif /* __AHCI_MULTIPLIER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
