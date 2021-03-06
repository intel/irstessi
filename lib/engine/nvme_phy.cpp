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
#include <log/log.h>

#include "nvme_phy.h"
#include "nvme_port.h"
#include "nvme_disk.h"

/* */
#define EM_MSG_WAIT     1500

using boost::shared_ptr;

/* */
NVME_Phy::NVME_Phy(const String &path, unsigned int number, const Parent& pParent)
    : Phy(path, number, pParent)
{
    m_Protocol = SSI_PhyProtocolNVME;
}

/* */
void NVME_Phy::discover()
{
    shared_ptr<NVME_Port> port = shared_ptr<NVME_Port>(new NVME_Port(m_Path));
    m_pPort = port;
    port->setParent(m_pParent.lock());
    port->attachPhy(shared_from_this());

    shared_ptr<EndDevice> pEndDevice = __internal_attach_end_device(m_Path);
    if (pEndDevice) {
        pEndDevice->setParent(m_pParent.lock());
        shared_ptr<Phy> pPhy = pEndDevice->getPhy();
        port->attachPort(pEndDevice->getPort());
        pPhy->setProtocol(m_Protocol);
    }

    if (Parent parent = m_pParent.lock()) {
        parent->attachPort(port);
    }
}

/* */
shared_ptr<EndDevice> NVME_Phy::__internal_attach_end_device(const String& path)
{
    shared_ptr<EndDevice> pEndDevice;
    try {
        pEndDevice = shared_ptr<EndDevice>(new NVME_Disk(path));
        pEndDevice->discover();
    } catch (...) {
        pEndDevice = shared_ptr<EndDevice>();
    }

    return pEndDevice;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
