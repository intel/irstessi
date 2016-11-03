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

/* */
AHCI_Phy::AHCI_Phy(const String &path, unsigned int number, StorageObject *pParent)
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
        m_pPort = new AHCI_Port(m_Path);
        m_pPort->setParent(m_pParent);
        m_pPort->attachPhy(this);

        if (dir.count() == 1) {
            EndDevice *pEndDevice = __internal_attach_end_device(*dir.dirs().begin());
            if (pEndDevice != NULL) {
                pEndDevice->setParent(m_pParent);
                Phy *pPhy = pEndDevice->getPhy();
                m_pPort->attachPort(pEndDevice->getPort());
                pPhy->setProtocol(m_Protocol);
            }
        } else {
            AHCI_Multiplier *pMultiplier = new AHCI_Multiplier(m_Path, dir);
            Phy *pPhy = pMultiplier->getPhy();
            pPhy->setProtocol(m_Protocol);
            pMultiplier->setParent(m_pParent);
            m_pPort->attachPort(pMultiplier->getSubtractivePort());
        }
        if (m_pParent != NULL) {
            m_pParent->attachPort(m_pPort);
        }
    }
}

/* */
EndDevice * AHCI_Phy::__internal_attach_end_device(Directory *dir)
{
    EndDevice *pEndDevice = NULL;
    std::list<Directory *> dirs = dir->dirs();
    foreach (j, dirs) {
        CanonicalPath temp = *(*j) + "driver";
        if (temp == "/sys/bus/scsi/drivers/sd") {
            pEndDevice = new AHCI_Disk(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/sr") {
            pEndDevice = new AHCI_CDROM(*(*j));
            break;
        } else
        if (temp == "/sys/bus/scsi/drivers/st") {
            pEndDevice = new AHCI_Tape(*(*j));
            break;
        }
    }
    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
