/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
        ISCI_Expander *pExpander = new ISCI_Expander(**dirs.begin());
        pExpander->setParent(m_pParent);
        pExpander->discover();
        attachPort(pExpander->getSubtractivePort());
        return;
    }
    dir.setFilter("end_device");
    dirs = dir.dirs();
    if (dirs.begin() != dirs.end()) {
        Directory target(**dirs.begin(), "target");
        StorageObject *pStorageObject = __internal_create_storage_object(target.dirs());
        if (pStorageObject != NULL) {
            pStorageObject->setParent(m_pParent);

            if (dynamic_cast<EndDevice *>(pStorageObject))
                attachPort(pStorageObject->getPort());
            else if (Enclosure * tmp = dynamic_cast<Enclosure *>(pStorageObject))
                attachEnclosure(tmp);
            else
                delete pStorageObject;
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
StorageObject * ISCI_Port::__internal_create_storage_object(std::list<Directory *> &dirs)
{
    StorageObject *pStorageObject = NULL;
    if (dirs.begin() != dirs.end()) {
        foreach (i, dirs) {
            CanonicalPath temp = *(*i) + "driver";
            if (temp == "/sys/bus/scsi/drivers/sd") {
                pStorageObject = new ISCI_Disk(*(*i));
                break;
            } else
            if (temp == "/sys/bus/scsi/drivers/sr") {
                pStorageObject = new ISCI_CDROM(*(*i));
                break;
            } else
            if (temp == "/sys/bus/scsi/drivers/st") {
                pStorageObject = new ISCI_Tape(*(*i));
                break;
            }
            if (temp == "/sys/bus/scsi/drivers/ses") {
                pStorageObject = new Enclosure(*(*i));

                break;
            }
        }
    }
    return pStorageObject;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
