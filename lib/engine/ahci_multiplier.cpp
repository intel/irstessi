
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
#include <stddef.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "storage_device.h"
#include "phy.h"
#include "port.h"
#include "end_device.h"
#include "nondisk_device.h"
#include "routing_device.h"
#include "session.h"
#include "block_device.h"
#include "multimedia_device.h"
#include "stream_device.h"
#include "ahci_disk.h"
#include "ahci_cdrom.h"
#include "ahci_tape.h"
#include "ahci_multiplier.h"
#include "ahci_phy.h"
#include "remote_port.h"

/* */
AHCI_Multiplier::AHCI_Multiplier(const String &path, Directory &dir)
    : RoutingDevice(path), m_pPhy(0)
{
    m_pPhy = new AHCI_Phy(path, 0, this);
    m_pPhy->setProtocol(SSI_PhyProtocolSATA);
    m_pSubtractivePort = new RemotePort(path);
    m_pSubtractivePort->setParent(this);
    m_pSubtractivePort->attachPhy(m_pPhy);

    unsigned int number = 0;
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        for (Iterator<Directory *> j = *(*i); *j != 0; ++j) {
            if (__internal_attach_end_device(*(*j), ++number)) {
                break;
            }
        }
    }
}

/* */
bool AHCI_Multiplier::__internal_attach_end_device(const Path &path, unsigned int number)
{
    CanonicalPath temp = path + "driver";
    EndDevice *pEndDevice = 0;
    if (temp == "/sys/bus/scsi/drivers/sd") {
        pEndDevice = new AHCI_Disk(path);
    } else
    if (temp == "/sys/bus/scsi/drivers/sr") {
        pEndDevice = new AHCI_CDROM(path);
    } else
    if (temp == "/sys/bus/scsi/drivers/st") {
        pEndDevice = new AHCI_Tape(path);
    }
    if (pEndDevice != 0) {
        Phy *pPhy = new Phy(path, number, this);
        attachPhy(pPhy);
        pPhy->setProtocol(SSI_PhyProtocolSATA);
        Port *pPort = new Port(path);
        attachPort(pPort);
        pPort->setParent(this);
        pPort->attachPhy(pPhy);
        pPort->attachPort(pEndDevice->getPort());
    }
    return (pEndDevice != 0);
}

/* */
void AHCI_Multiplier::acquireId(Session *pSession)
{
    pSession->addPhy(m_pPhy);
    RoutingDevice::acquireId(pSession);
}

/* */
void AHCI_Multiplier::getAddress(SSI_Address &address) const
{
    address.scsiAddress.host = 0;
    address.scsiAddress.bus = 0;
    address.scsiAddress.target = 0;
    address.scsiAddress.lun = 0;
    address.sasAddressPresent = SSI_FALSE;
    address.sasAddress = 0ULL;
}

/* */
void AHCI_Multiplier::getPhys(Container<Phy> &container) const
{
    RoutingDevice::getPhys(container);
    container.add(m_pPhy);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
