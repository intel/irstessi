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

#ifndef __CONTAINER_H__INCLUDED__
#define __CONTAINER_H__INCLUDED__

#include <cstdlib>
#include <list>
#include <boost/shared_ptr.hpp>
#include "utils.h"
#include <ssi.h>

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
template <typename T, typename SmartPtr = boost::shared_ptr<T> >
class Container {
public:
    typedef typename std::list<SmartPtr> list_type;
    typedef typename list_type::value_type value_type;
    typedef typename list_type::const_iterator const_iterator;

    SSI_Status getHandles(SSI_Handle *pBuffer, SSI_Uint32 *bufferSize) {
        SSI_Status status = SSI_StatusOk;

        if (bufferSize == NULL) {
            return SSI_StatusInvalidParameter;
        }

        if (*bufferSize && pBuffer == NULL) {
            return SSI_StatusInvalidParameter;
        }

        if (*bufferSize < m_list.size()) {
            *bufferSize = m_list.size();
            status = SSI_StatusBufferTooSmall;
        } else {
            foreach (i, m_list) {
                *pBuffer = (*i)->getHandle();
                pBuffer++;
            }
        }

        *bufferSize = m_list.size();
        return status;
    }

    value_type remove(SSI_Handle handle) {
        value_type pObject = find(handle);

        if (pObject) {
            m_list.remove(pObject);
        }

        return pObject;
    }

    value_type find(SSI_Handle handle) const {
        foreach (i, m_list) {
            if ((*i)->getHandle() == handle) {
                return *i;
            }
        }

        return value_type();
    }

    SSI_Status add(const value_type& data) {
        try {
            m_list.push_back(data);
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }

        return SSI_StatusOk;
    }

    SSI_Status add(const Container& c) {
        try {
            m_list.insert(m_list.end(), c.m_list.begin(), c.m_list.end());
        } catch (...) {
            return SSI_StatusInsufficientResources;
        }

        return SSI_StatusOk;
    }

    size_t size() const {
        return m_list.size();
    }

    const_iterator begin() const {
        return m_list.begin();
    }

    const_iterator end() const {
        return m_list.end();
    }

    const value_type& front() const {
        return m_list.front();
    }

    bool empty() const {
        return m_list.empty();
    }

    void clear() {
        m_list.clear();
    }

private:
    list_type m_list;
};

#endif /* __CONTAINER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
