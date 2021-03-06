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
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sstream>

extern "C" {
#include "lib/safeclib/safe_mem_lib.h"
}

#include "nvme_disk.h"
#include "filesystem.h"
#include "controller.h"
#include "volume.h"

using boost::shared_ptr;

namespace {
    /**
     * \brief Checks disk position in FDx8 by serial: XXXXXX-([1A]|[2B])
     * \param serial - serial of disk
     * \return 0 if it is not FDx8 disk, 1 or 2 if it is
     */
    unsigned int whichFultondalex8Disk(const String& serial)
    {
        if (serial.size() > 1) {
            if (serial[serial.size() - 2] == '-') {
                char last = serial[serial.size() - 1];

                if (last == 'A' || last == '1') {
                    return 1;
                } else if (last == 'B' || last == '2') {
                    return 2;
                }
            }
        }

        return 0;
    }

    template <typename T>
    void from_hex(const String& value, T& result)
    {
        unsigned int x;
        std::stringstream ss(value.get());
        ss >> std::hex >> x;
        result = static_cast<T>(x);
    }
}

void NVME_Disk::identify()
{
    struct nvme_admin_cmd cmd;
    struct nvme_id_ctrl ptr;

    int fd = 0;
    memset_s(&cmd, sizeof(cmd), 0);
    cmd.opcode = NVME_ADMIN_IDENTIFY_OPT_CODE;
    cmd.nsid = 0;
    cmd.addr = (unsigned long)&ptr;
    cmd.data_len = 4096;
    cmd.cdw10 = 1;
    fd = open("/dev/" + m_DevName, O_RDONLY);
    if (fd < 0) {
        return;
    }

    if (!ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd)) {
        char sn[sizeof(ptr.sn) + 1] = { 0 };
        char mn[sizeof(ptr.mn) + 1] = { 0 };
        char fr[sizeof(ptr.fr) + 1] = { 0 };

        memcpy_s(sn, sizeof(sn), ptr.sn, sizeof(ptr.sn));
        memcpy_s(mn, sizeof(mn), ptr.mn, sizeof(ptr.mn));
        memcpy_s(fr, sizeof(fr), ptr.fr, sizeof(ptr.fr));

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
}

/* */
void NVME_Disk::discover()
{
    const String IntelVendorTag = "8086";

    BlockDevice::discover();
    Directory dir("/sys/class/nvme");
    CanonicalPath temp;
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        temp = *(*i) + "device";
        if (temp == m_Path) {
            /* FIXME: search for other NVMe namespaces */
            m_DevName = (*i)->reverse_after("/") + "n1";
            break;
        }
    }

    /* clear scsi address for nvme since it's trash */
    m_SCSIAddress.host = 0;
    m_SCSIAddress.bus = 0;
    m_SCSIAddress.target = 0;
    m_SCSIAddress.lun = 0;

    String bdfAddress = m_Path.reverse_after("/");
    from_hex(bdfAddress.left(":"), m_BDFAddress.domain);
    m_vmdDomain = m_BDFAddress.domain & 0x0000F;

    bdfAddress = bdfAddress.after(":");
    from_hex(bdfAddress.left(":"), m_BDFAddress.bus);

    bdfAddress = bdfAddress.after(":");
    from_hex(bdfAddress.left("."), m_BDFAddress.device);

    bdfAddress = bdfAddress.after(".");
    from_hex(bdfAddress, m_BDFAddress.function);

    __internal_determine_disk_is_system();
    m_SerialNum = "";
    m_Model = "";
    m_Firmware = "";
    identify();

    m_SerialNum.trim();
    m_Model.trim();
    m_Firmware.trim();
    if (m_SerialNum.isEmpty() || m_Model.isEmpty() || m_Firmware.isEmpty()) {
        throw E_INVALID_OBJECT;
    }

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
        m_BlocksTotal = m_BlocksTotal / (m_LogicalSectorSize / DEFAULT_SECTOR_SIZE);
        m_TotalSize = (unsigned long long) m_BlocksTotal * m_LogicalSectorSize;
    } catch (...) {
    }

    String vendorId;
    try {
        attr = "/sys/class/block/" + m_DevName + "/device/device/vendor";
        attr >> vendorId;
    } catch (...) {
    }

    try {
        unsigned int pos = vendorId.find("0x");
        m_VendorId = vendorId.right(pos + sizeof("0x") - 1);
    } catch (...) {
        m_VendorId = vendorId;
    }

    m_BlocksFree = m_BlocksTotal;
    m_FDx8Disk = whichFultondalex8Disk(m_SerialNum);

    if (m_VendorId == IntelVendorTag) {
        m_isIntelNvme = true;
    }

    __internal_determine_disk_state();
    __internal_determine_disk_usage();
}

bool NVME_Disk::canRemoveDisk() const {
    Container<Volume> controllerVolumes;
    if (shared_ptr<Controller> controller = getController()) {
        if (controller->getHardwareMode() != SSI_HardwareKeyVROCStandard &&
            controller->getHardwareMode() != SSI_HardwareKeyVROCPremium &&
            !isIntelNvme()) {
            return false;
        }

        controller->getVolumes(controllerVolumes);
        Container<Volume> volumes;
        Container<EndDevice> devices;
        foreach (iter, controllerVolumes) {
            Volume& volume = *(*iter);

            volume.getEndDevices(devices, false);
            foreach (d, devices) {
                EndDevice& device = *(*d);

                if (device.getId() == getId()) {
                    volumes.add(*iter);
                    break;
                }
            }
        }

        if (!volumes.empty()) {
            foreach (iter, volumes) {
                Volume& volume = *(*iter);

                if (volume.isSystemVolume() &&
                    (volume.getSsiRaidLevel() == SSI_Raid0 ||
                     volume.getState() != SSI_VolumeStateNormal)) {
                    return false;
                }
            }

            return true;
        }
    }

    return !isSystemDisk();
}

SSI_Status NVME_Disk::removeDevice()
{
    if (sysfs_write("1", "/sys/class/block/" + m_DevName + "/device/device/remove") == 0) {
        return SSI_StatusOk;
    }

    return SSI_StatusFailed;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
