
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

#ifndef __LIST_H__INCLUDED__
#define __LIST_H__INCLUDED__

#include <list>

/* */
template <typename T> class Iterator {
public:
    bool operator == (const Iterator<T> &i) const {
        return m_iter == i.m_iter;
    }
    bool operator != (const Iterator<T> &i) const {
        return m_iter != i.m_iter;
    }
    void operator ++ () {
        ++m_iter;
    }
    const T operator * () const {
        return *m_iter;
    }

    template <typename K> friend class List;

private:
    typename std::list<T>::const_iterator m_iter;
};

/* */
template <typename T> class List {
public:
    List()
        : m_list() {
    }

public:

    int size() const {
        return m_list.size();
    }

    void add(const T &data) {
        m_list.push_back(data);
    }

    void add(const List<T> &list) {
        m_list.insert(m_list.end(), list.m_list.begin(), list.m_list.end());
    }

    void clear() {
        m_list.clear();
    }

    void remove(const T &pElem) {
        m_list.remove(pElem);
    }

    Iterator<T> begin() const {
        Iterator<T> i;
        i.m_iter = m_list.begin();
        return i;
    }

    Iterator<T> end() const {
        Iterator<T> i;
        i.m_iter = m_list.end();
        return i;
    }

protected:
    std::list<T> m_list;
};

#endif /* __LIST_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
