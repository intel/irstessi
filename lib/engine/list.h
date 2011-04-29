
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

/* */
template <typename T> class Element {
private:
    Element(const T &data)
        : m_Data(data), m_pNext(0), m_pPrev(0) {
    }
    ~Element() {
        if (m_pNext != 0) {
            delete m_pNext;
        }
        m_pNext = m_pPrev = 0;
    }

    template <typename K> friend class Iterator;
    template <typename K> friend class List;

private:
    const T m_Data;
    Element<T> *m_pNext;
    Element<T> *m_pPrev;
};

/* */
template <typename T> class Iterator {
public:
    Iterator(Element<T> *pElem = 0)
        : m_pElem(pElem) {
    }
    Iterator(const Iterator<T> &i)
        : m_pElem(i.m_pElem) {
    }

public:
    Element<T> * next() const {
        return m_pElem ? m_pElem->m_pNext : 0;
    }
    Element<T> * prev() const {
        return m_pElem ? m_pElem->m_pPrev : 0;
    }
    const T & data() const {
        return m_pElem ? m_pElem->m_Data  : 0;
    }

public:
    bool operator == (const Iterator<T> &i) const {
        return data() == i.data();
    }
    bool operator != (const Iterator<T> &i) const {
        return data() != i.data();
    }
    Iterator<T> operator ++ (int) {
        Iterator<T> result = *this;
        m_pElem = next();
        return result;
    }
    Iterator<T> & operator ++ () {
        m_pElem = next();
        return *this;
    }
    Iterator<T> operator -- (int) {
        Iterator<T> result = *this;
        m_pElem = prev();
        return result;
    }
    Iterator<T> & operator -- () {
        m_pElem = prev();
        return *this;
    }
    const T operator * () const {
        return m_pElem ? m_pElem->m_Data : 0;
    }
    operator Element<T> * () const {
        return m_pElem;
    }

    template <typename K> friend class List;

private:
    Element<T> *m_pElem;
};

/* */
template <typename T> class List {
public:
    List()
        : m_pHead(0), m_pTail(0), m_Count(0) {
    }
    List(const List<T> &list)
        : m_pHead(0), m_pTail(0), m_Count(0) {
        add(list);
    }
    virtual ~List() {
        delete m_pHead;
    }

public:
    unsigned int count() const {
        return m_Count;
    }
    void add(const List<T> &list) {
        for (Iterator<T> i = list; *i != 0; ++i) {
            add(*i);
        }
    }
    void clear() {
        delete m_pHead;
        m_pHead = m_pTail = 0;
        m_Count = 0;
    }
    void add(const T &data) {
        Element<T> *pElem = new Element<T>(data);
        if (m_pHead == 0) {
            m_pHead = pElem;
        } else {
            m_pTail->m_pNext = pElem;
            pElem->m_pPrev = m_pTail;
        }
        m_Count++;
        m_pTail = pElem;
    }
    T remove(const T &data) {
        Element<T> *pElem = m_pHead;
        while (pElem != 0) {
            if (pElem->m_Data == data) {
                break;
            }
            pElem = pElem->m_pNext;
        }
        return remove(pElem);
    }
    T remove(Element<T> *pElem) {
        if (pElem == 0) {
            throw E_NULL_POINTER;
        }
        T result = pElem->m_Data;
        if (pElem->m_pPrev) {
            pElem->m_pPrev->m_pNext = pElem->m_pNext;
        } else {
            m_pHead = pElem->m_pNext;
        }
        if (pElem->m_pNext) {
            pElem->m_pNext->m_pPrev = pElem->m_pPrev;
        } else {
            m_pTail = pElem->m_pPrev;
        }
        m_Count--;
        pElem->m_pNext = 0;
        delete pElem;
        return result;
    }
    Iterator<T> first() const {
        return Iterator<T>(m_pHead);
    }
    Iterator<T> last() const {
        return Iterator<T>(m_pTail);
    }
    operator Iterator<T> () const {
        return first();
    }
    operator unsigned int () const {
        return m_Count;
    }

public:
    List<T> & operator = (const List<T> &list) {
        clear(); add(list); return *this;
    }

protected:
    Element<T> *m_pHead;
    Element<T> *m_pTail;
    unsigned int m_Count;
};

template <typename T>
inline List<T> operator + (const List<T> &left, const List<T> &right) {
    return List<T>(left).add(right);
}

#endif /* __LIST_H__INCLUDED__ */

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
