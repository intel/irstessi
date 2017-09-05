/*
 * Copyright 2011 - 2017 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __RAID_INFO_H__INCLUDED__
#define __RAID_INFO_H__INCLUDED__

#include "scope_object.h"
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
class RaidInfo : public ScopeObject, public boost::enable_shared_from_this<RaidInfo> {
public:
    RaidInfo(struct orom_info *pInfo);

    // Object

public:
    virtual String getId() const;

    // ScopeObject

public:
    virtual void getControllers(Container<Controller> &) const;

    virtual bool scopeTypeMatches(SSI_ScopeType scopeType) const {
        return scopeType == SSI_ScopeTypeRaidInfo;
    }

    // RaidInfo

public:
    boost::weak_ptr<Controller> m_Controller;
    unsigned int m_OromDevId;
    struct orom_info * m_pInfo;

public:
    virtual void attachController(const boost::shared_ptr<Controller>& pController) {
        m_Controller = pController;
    }
    virtual void addToSession(const boost::shared_ptr<Session>& pSession);

    virtual SSI_ControllerType getControllerType() const = 0;
    SSI_Status getRaidLevelInfo(SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *pInfo) const;
    virtual SSI_Status getInfo(SSI_RaidInfo *info) const;
};

#endif /* __RAID_INFO_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
