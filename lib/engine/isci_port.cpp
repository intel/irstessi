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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <cstddef>
#include <log/log.h>

#include "isci_port.h"
#include "isci_expander.h"
#include "isci_disk.h"
#include "isci_cdrom.h"
#include "isci_tape.h"
#include "enclosure.h"

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

/* */
ISCI_Port::ISCI_Port(const String &path)
    : Port(path)
{
}

/* */
void ISCI_Port::discover()
{
    Directory dir(m_Path, "expander");
    std::list<Directory *> dirs = dir.dirs();
    if (dirs.begin() != dirs.end()) {
        shared_ptr<ISCI_Expander> pExpander = shared_ptr<ISCI_Expander>(new ISCI_Expander(**dirs.begin()));
        pExpander->setParent(m_pParent.lock());
        pExpander->discover();
        attachPort(pExpander->getSubtractivePort());
        return;
    }

    dir.setFilter("end_device");
    dirs = dir.dirs();
    if (dirs.begin() != dirs.end()) {
        Directory target(**dirs.begin(), "target");
        shared_ptr<StorageObject> pStorageObject = __internal_create_storage_object(target.dirs());
        if (pStorageObject) {
            pStorageObject->setParent(m_pParent.lock());

            if (dynamic_cast<EndDevice *>(pStorageObject.get())) {
                attachPort(pStorageObject->getPort());
            } else if (shared_ptr<Enclosure> tmp = dynamic_pointer_cast<Enclosure>(pStorageObject)) {
                attachEnclosure(tmp);
            }
        }

        return;
    }

    dir.setFilter("host");
    dirs = dir.dirs();
    if (dirs.begin() == dirs.end()) {
        throw E_INVALID_OBJECT;
    }
}

/* */
shared_ptr<StorageObject> ISCI_Port::__internal_create_storage_object(std::list<Directory *> &dirs)
{
    shared_ptr<StorageObject> pStorageObject;
    if (dirs.begin() != dirs.end()) {
        foreach (i, dirs) {
            CanonicalPath temp = *(*i) + "driver";
            if (temp == "/sys/bus/scsi/drivers/sd") {
                pStorageObject = shared_ptr<StorageObject>(new ISCI_Disk(*(*i)));
                break;
            } else if (temp == "/sys/bus/scsi/drivers/sr") {
                pStorageObject = shared_ptr<StorageObject>(new ISCI_CDROM(*(*i)));
                break;
            } else if (temp == "/sys/bus/scsi/drivers/st") {
                pStorageObject = shared_ptr<StorageObject>(new ISCI_Tape(*(*i)));
                break;
            } else if (temp == "/sys/bus/scsi/drivers/ses") {
                pStorageObject = shared_ptr<StorageObject>(new Enclosure(*(*i)));
                break;
            }
        }
    }

    if (shared_ptr<EndDevice> endDevice = dynamic_pointer_cast<EndDevice>(pStorageObject)) {
        endDevice->discover();
    }

    return pStorageObject;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
