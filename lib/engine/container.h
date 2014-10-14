
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if __GNUC_PREREQ(3,4)
#pragma once
#endif /* __GNUC_PREREQ */

#ifndef __CONTAINER_H__INCLUDED__
#define __CONTAINER_H__INCLUDED__

#include <cstdlib>
#include "list.h"

/* */
template <typename T>
class Container {
public:
    SSI_Status getHandles(SSI_Handle *pBuffer, SSI_Uint32 *bufferSize) {
        SSI_Status status = SSI_StatusOk;
        if (bufferSize == 0) {
            return SSI_StatusInvalidParameter;
        }
        if (*bufferSize && pBuffer == 0) {
            return SSI_StatusInvalidParameter;
        }
        if (*bufferSize < _list.size()) {
            *bufferSize = _list.size();
            status = SSI_StatusBufferTooSmall;
        } else {
            for (typename std::list<T *>::const_iterator i = _list.begin(); i != _list.end(); ++i, ++pBuffer) {
                *pBuffer = (*i)->getId();
            }
        }
        *bufferSize = _list.size();
        return status;
    }

    T * remove(unsigned int id) {
        T *pObject = find(id);

        if (pObject) {
            _list.remove(pObject);
        }
        return pObject;
    }

    T * find(unsigned int id) const {
        typename std::list<T *>::const_iterator i;
        for (i = _list.begin(); i != _list.end() && (*i)->getId() != id; ++i) {
        }

        if (i == _list.end())
            return NULL;

        return *i;
    }

    void add(T * const &data) {
        _list.push_back(data);
    }

    void add(Container<T> const &c) {
        _list.insert(_list.end(), c._list.begin(), c._list.end());
    }

    int size() const {
        return _list.size();
    }

    typename std::list<T *>::const_iterator begin() const {
        return _list.begin();
    }

    typename std::list<T *>::const_iterator end() const {
        return _list.end();
    }

    void clear() {
        _list.clear();
    }

private:
    std::list<T *> _list;
};

#endif /* __CONTAINER_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
