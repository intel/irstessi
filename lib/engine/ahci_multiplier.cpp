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

#include "ahci_multiplier.h"

#include "session.h"
#include "remote_port.h"
#include "ahci_phy.h"
#include "ahci_disk.h"
#include "ahci_cdrom.h"
#include "ahci_tape.h"

using boost::shared_ptr;

/* */
AHCI_Multiplier::AHCI_Multiplier(const String &path, Directory &dir)
    : RoutingDevice(path), m_pPhy(), m_dir(dir)
{

}

/* */
void AHCI_Multiplier::discover()
{
    m_pPhy = shared_ptr<Phy>(new AHCI_Phy(m_Path, 0, shared_from_this()));
    m_pPhy->setProtocol(SSI_PhyProtocolSATA);
    m_pSubtractivePort = shared_ptr<RemotePort>(new RemotePort(m_Path));
    m_pSubtractivePort->setParent(shared_from_this());
    m_pSubtractivePort->attachPhy(m_pPhy);

    unsigned int number = 0;
    std::list<Directory *> dirs = m_dir.dirs();
    foreach (i, dirs) {
        std::list<Directory *> dirs2 = (*i)->dirs();
        foreach (j, dirs2) {
            if (__internal_attach_end_device(*(*j), ++number)) {
                break;
            }
        }
    }
}

/* */
AHCI_Multiplier::AHCI_Multiplier(const AHCI_Multiplier &multiplier)
    : RoutingDevice(multiplier.getPath())
{
    /* do not create copies */
}

/* */
bool AHCI_Multiplier::__internal_attach_end_device(const Path &path, unsigned int number)
{
    CanonicalPath temp = path + "driver";
    shared_ptr<EndDevice> pEndDevice;
    if (temp == "/sys/bus/scsi/drivers/sd") {
        pEndDevice = shared_ptr<EndDevice>(new AHCI_Disk(path));
    } else if (temp == "/sys/bus/scsi/drivers/sr") {
        pEndDevice = shared_ptr<EndDevice>(new AHCI_CDROM(path));
    } else if (temp == "/sys/bus/scsi/drivers/st") {
        pEndDevice = shared_ptr<EndDevice>(new AHCI_Tape(path));
    }

    if (pEndDevice) {
        pEndDevice->discover();
        shared_ptr<Phy> pPhy = shared_ptr<Phy>(new Phy(path, number, shared_from_this()));
        attachPhy(pPhy);
        pPhy->setProtocol(SSI_PhyProtocolSATA);
        shared_ptr<Port> pPort = shared_ptr<Port>(new Port(path));
        attachPort(pPort);
        pPort->setParent(shared_from_this());
        pPort->attachPhy(pPhy);
        pPort->attachPort(pEndDevice->getPort());

        return true;
    } else {
        return false;
    }
}

/* */
void AHCI_Multiplier::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addPhy(m_pPhy);
    RoutingDevice::addToSession(pSession);
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
