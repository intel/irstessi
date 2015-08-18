
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
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <ssi.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "raid_info.h"
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
#include "nvme_phy.h"
#include "nvme_port.h"
#include "nvme_disk.h"
#include "utils.h"

#include "log/log.h"

/* */
#define EM_MSG_WAIT     1500

/* */
NVME_Phy::NVME_Phy(const String &path, unsigned int number, StorageObject *pParent)
    : Phy(path, number, pParent)
{
	m_Protocol = SSI_PhyProtocolNVME;
}

/* */
void NVME_Phy::discover()
{
    m_pPort = new NVME_Port(m_Path);
    m_pPort->setParent(m_pParent);
    m_pPort->attachPhy(this);

    EndDevice *pEndDevice = __internal_attach_end_device(m_Path);
    if (pEndDevice != NULL) {
        pEndDevice->setParent(m_pParent);
        Phy *pPhy = pEndDevice->getPhy();
        m_pPort->attachPort(pEndDevice->getPort());
        pPhy->setProtocol(m_Protocol);
    }
    if (m_pParent != NULL) {
        m_pParent->attachPort(m_pPort);
    }
}

/* */
EndDevice * NVME_Phy::__internal_attach_end_device(String path)
{
    EndDevice *pEndDevice = NULL;
    pEndDevice = new NVME_Disk(path);
    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */