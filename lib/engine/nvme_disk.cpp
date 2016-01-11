
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
#include <cstddef>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include <ssi.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "storage_device.h"
#include "end_device.h"
#include "block_device.h"
#include "nvme_disk.h"



void NVME_Disk::identify()
{
    struct nvme_admin_cmd cmd;
    struct nvme_id_ctrl ptr;

    int fd=0;
    memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = NVME_ADMIN_IDENTIFY_OPT_CODE;
    cmd.nsid = 0;
    cmd.addr = (unsigned long)&ptr;
    cmd.data_len = 4096;
    cmd.cdw10 = 1;
    fd = open("/dev/" + m_DevName, O_RDONLY);
    if(fd < 0)
        return;
    if (!ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd)){
        char sn[sizeof(ptr.sn) + 1];
        char mn[sizeof(ptr.mn) + 1];
        char fr[sizeof(ptr.fr) + 1];

        memcpy(sn, ptr.sn, sizeof(ptr.sn));
        memcpy(mn, ptr.mn, sizeof(ptr.mn));
        memcpy(fr, ptr.fr, sizeof(ptr.fr));

        sn[sizeof(ptr.sn)] = '\0';
        mn[sizeof(ptr.mn)] = '\0';
        fr[sizeof(ptr.fr)] = '\0';

        m_SerialNum = sn;
        m_Model = mn;
        m_Firmware = fr;
    }
    close(fd);
}

/* */
NVME_Disk::NVME_Disk(const String &path)
    : BlockDevice(path)
{
    Directory dir("/sys/class/nvme");
    CanonicalPath temp;
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        temp = *(*i) + "device";
        if (temp == m_Path) {
            //FIXME: search for other NVMe namespaces
            m_DevName = (*i)->reverse_after("/") + "n1";
            break;
        }
    }

    identify();
    File attr;

    try {
        attr = "/sys/class/block/" + m_DevName + "/queue/logical_block_size";
        attr >> m_LogicalSectorSize;
    } catch (...) {
    }

    try {
        attr = "/sys/class/block/" + m_DevName + "/queue/physical_block_size";
        attr >> m_PhysicalSectorSize;
    } catch (...) {
    }

    try {
        attr =  "/sys/class/block/" + m_DevName + "/size";
        attr >> m_BlocksTotal;
        m_TotalSize = (unsigned long long) m_BlocksTotal * m_LogicalSectorSize;
    } catch (...) {
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
