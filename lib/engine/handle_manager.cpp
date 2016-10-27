/*
Copyright (c) 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <limits>
#include <features.h>
#include <cstdlib>

#include <ssi.h>

#include "exception.h"
#include "container.h"
#include "object.h"
#include "handle_manager.h"

using std::pair;
using std::make_pair;
using std::numeric_limits;

pair<SSI_Handle, bool> HandleManager::insert(Object *object)
{
    SSI_Handle unique = getUniqueHandle(object);
    if (unique == SSI_NULL_HANDLE) {
        return make_pair(SSI_NULL_HANDLE, false);
    }

    pair<iterator, bool> ret;
    try {
        ret = m_objects.insert(make_pair(unique, object));
    } catch (...) {
        return make_pair(SSI_NULL_HANDLE, false);
    }

    if (ret.second) { /* Object was inserted */
        object->setHandle(ret.first->first);
    }

    return make_pair(ret.first->first, ret.second);
}

Object* HandleManager::remove(SSI_Handle handle)
{
    if (handle != SSI_NULL_HANDLE) {
        iterator found = m_objects.find(handle);
        if (found != m_objects.end()) {
            Object* object = found->second;
            object->setHandle(SSI_NULL_HANDLE);
            m_objects.erase(found);

            return object;
        }
    }

    return NULL;
}

void HandleManager::remove(Object *object)
{
    remove(object->getHandle());
}

Object* HandleManager::operator[](SSI_Handle handle)
{
    if (handle == SSI_NULL_HANDLE) {
        return NULL;
    } else {
        try {
            return m_objects[handle];
        } catch (...) {
            return NULL;
        }
    }
}

Object* HandleManager::at(SSI_Handle handle)
{
    if (handle == SSI_NULL_HANDLE) {
        return NULL;
    }

    try {
        return m_objects.at(handle);
    } catch (...) {
        return NULL;
    }
}

const Object* HandleManager::at(SSI_Handle handle) const
{
    if (handle == SSI_NULL_HANDLE) {
        return NULL;
    }

    try {
        return m_objects.at(handle);
    } catch (...) {
        return NULL;
    }
}

std::size_t HandleManager::size() const
{
    return m_objects.size();
}

HandleManager::iterator HandleManager::begin()
{
    return m_objects.begin();
}

HandleManager::iterator HandleManager::end()
{
    return m_objects.end();
}

HandleManager::const_iterator HandleManager::begin() const
{
    return m_objects.begin();
}

HandleManager::const_iterator HandleManager::end() const
{
    return m_objects.end();
}

SSI_Handle HandleManager::getUniqueHandle(Object *object) const
{
    const SSI_Handle initValue = 1;

    if (at(object->getHandle()) != NULL) {
        return object->getHandle();
    } else if (!m_objects.empty()) {
        const_iterator last = --m_objects.end();

        if (last->first == numeric_limits<SSI_Handle>::max()) {
            return SSI_NULL_HANDLE; /* Out of resources */
        }

        return last->first + 1;
    } else {
        return initValue;
    }
}
