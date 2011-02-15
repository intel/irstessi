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
