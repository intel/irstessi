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

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ahci_phy.h"
#include "ahci_port.h"
#include "ahci_disk.h"
#include "ahci_cdrom.h"
#include "ahci_tape.h"
#include "ahci_multiplier.h"

#include "log/log.h"

/* */
#define EM_MSG_WAIT     1500

using boost::shared_ptr;

/* */
AHCI_Phy::AHCI_Phy(const String &path, unsigned int number, const Parent& pParent)
    : Phy(path, number, pParent), m_PhyPath(CanonicalPath(path + "/scsi_host" + path.reverse_right("/host")))
{
    m_Protocol = SSI_PhyProtocolSATA;
    try {
        File attr = m_PhyPath + "/unique_id";
        String id;
        attr >> id;
        dlog(id);
        try {
            attr = path + "/../../ata" + id + "/link" + id + "/ata_link/link" + id + "/sata_spd";
            dlog(attr);
            attr >> id;
            dlog(id);
            m_negotiatedLinkSpeed = __internal_parse_linkrate(id);
        } catch (...) {
        }
    } catch (...) {
    }
    dlog(String(m_negotiatedLinkSpeed));
}

/* */
void AHCI_Phy::discover()
{
    Directory dir(m_Path, "target");
    if (dir > 0) {
        shared_ptr<Port> port = shared_ptr<Port>(new AHCI_Port(m_Path));
        m_pPort = port;
        port->setParent(m_pParent.lock());
        port->attachPhy(shared_from_this());

        if (dir.count() == 1) {
            shared_ptr<EndDevice> pEndDevice = __internal_attach_end_device(*dir.dirs().begin());
            if (pEndDevice) {
                pEndDevice->setParent(m_pParent.lock());
                shared_ptr<Phy> pPhy = pEndDevice->getPhy();
                port->attachPort(pEndDevice->getPort());
                pPhy->setProtocol(m_Protocol);
            }
        } else {
            shared_ptr<AHCI_Multiplier> pMultiplier = shared_ptr<AHCI_Multiplier>(new AHCI_Multiplier(m_Path, dir));
            pMultiplier->discover();
            shared_ptr<Phy> pPhy = pMultiplier->getPhy();
            pPhy->setProtocol(m_Protocol);
            pMultiplier->setParent(m_pParent.lock());
            port->attachPort(pMultiplier->getSubtractivePort());
        }

        if (Parent parent = m_pParent.lock()) {
            parent->attachPort(port);
        }
    }
}

/* */
shared_ptr<EndDevice> AHCI_Phy::__internal_attach_end_device(Directory *dir)
{
    std::list<Directory *> dirs = dir->dirs();
    shared_ptr<EndDevice> pEndDevice;
    foreach (j, dirs) {
        CanonicalPath temp = *(*j) + "driver";
        if (temp == "/sys/bus/scsi/drivers/sd") {
            pEndDevice = shared_ptr<EndDevice>(new AHCI_Disk(*(*j)));
            break;
        } else if (temp == "/sys/bus/scsi/drivers/sr") {
            pEndDevice = shared_ptr<EndDevice>(new AHCI_CDROM(*(*j)));
            break;
        } else if (temp == "/sys/bus/scsi/drivers/st") {
            pEndDevice = shared_ptr<EndDevice>(new AHCI_Tape(*(*j)));
            break;
        }
    }

    if (pEndDevice) {
        pEndDevice->discover();
    }

    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
