
/*
Copyright (c) 2011, Intel Corporation
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

#include <features.h>
#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "raid_info.h"
#include "cache.h"
#include "controller.h"
#include "phy.h"
#include "session.h"
#include "port.h"
#include "storage_device.h"
#include "end_device.h"
#include "nondisk_device.h"
#include "block_device.h"
#include "multimedia_device.h"
#include "stream_device.h"
#include "routing_device.h"
#include "isci.h"
#include "isci_port.h"
#include "isci_phy.h"
#include "isci_expander.h"
#include "isci_disk.h"
#include "isci_cdrom.h"
#include "isci_tape.h"

/* */
ISCI_Port::ISCI_Port(const String &path)
    : Port(path)
{
}

/* */
void ISCI_Port::discover()
{
    Iterator<Directory *> i;
    Directory dir(m_Path, "expander");
    if (*(i = dir) != 0) {
        ISCI_Expander *pExpander = new ISCI_Expander(*(*i));
        pExpander->discover();
        attachPort(pExpander->getSubtractivePort());
        return;
    }
    dir.setFilter("end_device");
    if (*(i = dir) != 0) {
        Directory target(*(*i), "target");
        EndDevice *pEndDevice = __internal_create_end_device(target);
        if (pEndDevice != 0) {
            attachPort(pEndDevice->getPort());
        }
        return;
    }
    dir.setFilter("host");
    if (*(i = dir) == 0) {
        throw E_INVALID_OBJECT;
    }
}

/* */
EndDevice * ISCI_Port::__internal_create_end_device(Iterator<Directory *> i)
{
    EndDevice *pEndDevice = 0;
    if (*i != 0)
    for (Iterator<Directory *> j = *(*i); *j != 0; ++j) {
        CanonicalPath temp = *(*j) + "driver";
        if (temp == "/sys/bus/scsi/drivers/sd") {
            pEndDevice = new ISCI_Disk(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/sr") {
            pEndDevice = new ISCI_CDROM(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/st") {
            pEndDevice = new ISCI_Tape(*(*j));
            break;
        }
    }
    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
