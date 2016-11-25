/*
Copyright (c) 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __HANDLE_MANAGER_H__INCLUDED__
#define __HANDLE_MANAGER_H__INCLUDED__

#include <map>
#include <limits>
#include <boost/shared_ptr.hpp>

#include "object.h"

#ifdef SSI_HAS_PRAGMA_ONCE
#pragma once
#endif

/* */
template <typename T>
class HandleManager {
public:
    typedef boost::shared_ptr<T> object_ptr;
    typedef std::map<SSI_Handle, object_ptr> map_type;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;

    std::pair<SSI_Handle, bool> insert(const object_ptr& object)
    {
        SSI_Handle unique = getUniqueHandle(object.get());
        if (unique == SSI_NULL_HANDLE) {
            return std::make_pair(SSI_NULL_HANDLE, false);
        }

        std::pair<iterator, bool> ret;
        try {
            ret = m_objects.insert(std::make_pair(unique, object));
        } catch (...) {
            return std::make_pair(SSI_NULL_HANDLE, false);
        }

        if (ret.second) { /* Object was inserted */
            object->setHandle(ret.first->first);
        }

        return std::make_pair(ret.first->first, ret.second);
    }

    object_ptr remove(T *object)
    {
        return remove(object->getHandle());
    }

    object_ptr remove(SSI_Handle handle)
    {
        if (handle != SSI_NULL_HANDLE) {
            iterator found = m_objects.find(handle);
            if (found != m_objects.end()) {
                object_ptr object = found->second;
                object->setHandle(SSI_NULL_HANDLE);
                m_objects.erase(found);

                return object;
            }
        }

        return object_ptr();
    }

    object_ptr operator[](SSI_Handle handle)
    {
        if (handle == SSI_NULL_HANDLE) {
            return object_ptr();
        } else {
            try {
                return m_objects[handle];
            } catch (...) {
                return object_ptr();
            }
        }
    }

    object_ptr at(SSI_Handle handle)
    {
        if (handle == SSI_NULL_HANDLE) {
            return object_ptr();
        }

        try {
            return m_objects.at(handle);
        } catch (...) {
            return object_ptr();
        }
    }

    const object_ptr at(SSI_Handle handle) const
    {
        if (handle == SSI_NULL_HANDLE) {
            return object_ptr();
        }

        try {
            return m_objects.at(handle);
        } catch (...) {
            return object_ptr();
        }
    }

    std::size_t size() const
    {
        return m_objects.size();
    }

    iterator begin()
    {
        return m_objects.begin();
    }

    iterator end()
    {
        return m_objects.end();
    }

    const_iterator begin() const
    {
        return m_objects.begin();
    }

    const_iterator end() const
    {
        return m_objects.end();
    }

private:
    SSI_Handle getUniqueHandle(T *object) const
    {
        const SSI_Handle initValue = 1;

        if (at(object->getHandle())) {
            return object->getHandle();
        } else if (!m_objects.empty()) {
            const_iterator last = --m_objects.end();

            if (last->first == std::numeric_limits<SSI_Handle>::max()) {
                return SSI_NULL_HANDLE; /* Out of resources */
            }

            return last->first + 1;
        } else {
            return initValue;
        }
    }

    map_type m_objects;
};

#endif /* __HANDLE_MANAGER_H__INCLUDED__ */
