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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/hdreg.h>
#include <unistd.h>
#include <linux/fs.h>
#include <climits>
#include <memory.h>
#include <typeinfo>
#include <vector>
#include <fstream>
#include <scsi/sg_lib.h>
#include <scsi/sg_cmds_basic.h>

extern "C" {
#include "lib/safeclib/safe_mem_lib.h"
}

#include "end_device.h"
#include "phy.h"
#include "remote_port.h"
#include "filesystem.h"
#include "enclosure.h"
#include "controller.h"
#include "raid_info.h"
#include "array.h"
#include "session.h"
#include "volume.h"

using std::vector;
using std::ifstream;

/* */
#define HD_SERIALNO_LENGTH              20

/* */
#define SCSI_INQUIRY_MODEL_OFFSET       16
#define SCSI_INQUIRY_FW_OFFSET          32
#define SCSI_INQUIRY_SERIALNUM_OFFSET   36

/* */
#define SCSI_SENSE_DISK_CACHE_OFFSET    16
#define SCSI_SENSE_DISK_CACHE_MASK      4
#define SCSI_SENSE_DISK_SETTINGS_PAGE   8

/* */
#define SCSI_CAP_BUFFER_SIZE            8
#define SCSI_SENSE_DISK_SETTING_PAGE    8
#define HD_DATA_BUFFER_SIZE             24

namespace {
    vector<String> split(const String& string, const String& delimiter)
    {
        bool found = true;
        vector<String> splitted;
        unsigned int offset = 0;
        while (found) {
            unsigned int pos;
            try {
                pos = string.find(delimiter, offset);
            } catch (...) {
                found = false;
                continue;
            }

            splitted.push_back(string.mid(offset, pos));
            offset = pos + 1;
        }

        return splitted;
    }
} // <<anonymous>>

EndDevice::EndDevice(const EndDevice &endDevice)
    : StorageDevice(endDevice.getPath())
{
   /* do not create copies */
}

/* */
EndDevice::EndDevice(const String &path)
    : StorageDevice(path),
      m_VendorId(""),
      m_SerialNum(""),
      m_pPhy(NULL),
      m_pPort(NULL),
      m_pEnclosure(NULL),
      m_Model(""),
      m_Firmware(""),
      m_TotalSize(0),
      m_BlocksTotal(0),
      m_BlocksFree(0),
      m_LogicalSectorSize(0),
      m_PhysicalSectorSize(0),
      m_SASAddress(0),
      m_WriteCachePolicy(SSI_WriteCachePolicyOff),
      m_ledState(0),
      m_systemIoBusNumer(0),
      m_PCISlotNumber(0),
      m_FDx8Disk(0),
      m_vmdDomain(0),
      m_isIntelNvme(false)
{
    m_pPhy = new Phy(path, 0, this);
    m_pPort = new RemotePort(path);
    m_pPort->setParent(this);
    m_pPort->attachPhy(m_pPhy);

    String scsiAddress = m_Path.reverse_after("/");

    m_SCSIAddress.host = scsiAddress;
    scsiAddress = scsiAddress.after(":");
    m_SCSIAddress.bus = scsiAddress;
    scsiAddress = scsiAddress.after(":");
    m_SCSIAddress.target = scsiAddress;
    scsiAddress = scsiAddress.after(":");
    m_SCSIAddress.lun = scsiAddress;

    // BDF address is currently used only for NVMe/VMD
    m_BDFAddress.domain = 0;
    m_BDFAddress.bus = 0;
    m_BDFAddress.device = 0;
    m_BDFAddress.function = 0;

    Directory dir(m_Path + "/block");
    CanonicalPath temp;
    std::list<Directory *> dirs = dir.dirs();
    foreach (i, dirs) {
        temp = *(*i) + "device";
        if (temp == m_Path) {
            m_DevName = (*i)->reverse_after("/");
            break;
        }
    }

    if (!m_DevName) {
        m_DevName = m_Path.reverse_after("/");
    }

    if (m_DevName.isEmpty()) {
        return;
    }

    temp = m_Path + "/generic";
    m_SgName = temp.reverse_after("/");
    File attr;

    try {
        attr = m_Path + "/vendor";
        attr >> m_Model;
    } catch (...) {
    }
    try {
        attr = m_Path + "/model";
        String tmp;
        attr >> tmp;
        if (m_Model == "ATA")
            m_Model = tmp;
        else
            m_Model.append(" " + tmp);
    } catch (...) {
    }

    try {
        attr = m_Path + "/rev";
        attr >> m_Firmware;
    } catch (...) {
    }

    try {
        SysfsAttr attr = "/sys/class/block/" + m_DevName + "/queue/logical_block_size";
        attr >> m_LogicalSectorSize;
    } catch (...) {
    }

    try {
        SysfsAttr attr = "/sys/class/block/" + m_DevName + "/queue/physical_block_size";
        attr >> m_PhysicalSectorSize;
    } catch (...) {
    }

    try {
        attr =  "/sys/class/block/" + m_DevName + "/size";
        attr >> m_BlocksTotal;
        m_BlocksTotal /= (m_LogicalSectorSize / DEFAULT_SECTOR_SIZE);
        m_TotalSize = (unsigned long long) m_BlocksTotal * m_LogicalSectorSize;
    } catch (...) {
    }

    getAtaDiskInfo("/dev/"+ m_DevName, m_Model, m_SerialNum, m_Firmware);

    unsigned char buffer[4096];
    bool hdioNotSupported = true;

    int fd = open("/dev/" + m_DevName, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        struct hd_driveid id;
        if (ioctl(fd, HDIO_GET_IDENTITY, &id) >= 0) {
            if (m_SerialNum.isEmpty()) {
                m_SerialNum.assign(reinterpret_cast<char*>(id.serial_no), sizeof(id.serial_no));
                m_SerialNum.trim();
            }

            if ((id.command_set_1 & id.cfs_enable_1) != 0) {
                m_WriteCachePolicy = SSI_WriteCachePolicyOn;
            }

            hdioNotSupported = false;
        }
        close(fd);
    }
    fd = sg_cmds_open_device("/dev/" + m_SgName, 0, 0);
    if (fd >= 0) {
        if (hdioNotSupported) {
            String sbuffer;
            const String command = "sg_inq /dev/" + m_DevName;
            if (m_SerialNum.isEmpty() && shell_cap(command, sbuffer) == 0) {
                m_SerialNum = sbuffer.between("Unit serial number: ", "\n");
                m_SerialNum.trim();
            }
            if (sg_ll_mode_sense10(fd, 0, 0, 0, SCSI_SENSE_DISK_SETTINGS_PAGE, 0, buffer, sizeof(buffer), 1, 0) == 0) {
                if ((*(buffer + SCSI_SENSE_DISK_CACHE_OFFSET) & SCSI_SENSE_DISK_CACHE_MASK) != 0) {
                    m_WriteCachePolicy = SSI_WriteCachePolicyOn;
                }
            }
        }
        sg_cmds_close_device(fd);
    }
}

/**
 * @brief Fills model, vendor and firmware properties for ATA drives using smartctl
 *
 * @return  0 for success, -1 if popen fails and status of smartctl otherwise
 */
int EndDevice::getAtaDiskInfo(const String &devName, String &model, String &serial, String &firmware)
{
    const char Delimiter[] = "\n";
    const unsigned int ModelIndex = 0;
    const unsigned int SerialIndex = 1;
    const unsigned int FirmwareIndex = 2;

    // SSI sometimes uses m_devName as handler and for some cases (like VMD) '/dev/' + m_devName points to nothing
    if (!ifstream(devName)) {
        return -1;
    }

    String data;

    /* We use smartctl to get info about device,
     * then grep only leaves model, revision and serial lines (in such order) and
     * then cut edits "[KEY]:   [VALUE]" to "   [VALUE]" for each line (trim is required) */
    const String command = "smartctl -i " + devName + " | grep -E 'Device Model|Serial Number|Firmware Version' | cut -d : -f 2-";
    int status = shell_output(command, data);
    if (status != 0) {
        return status;
    }

    vector<String> values = split(data, Delimiter);

    if (values.size() == 3) {
        foreach (value, values) {
            (*value).trim();
        }
        model = values[ModelIndex];
        serial = values[SerialIndex];
        firmware = values[FirmwareIndex];
    } else {
        status = -1;
    }

    return status;
}

/* */
EndDevice::~EndDevice()
{
    if (m_pPhy) {
        delete m_pPhy;
    }
    if (m_pPort) {
        delete m_pPort;
    }
    if (m_pEnclosure) {
        delete m_pEnclosure;
    }
}

String EndDevice::getId() const
{
    return "ed:" + getPartId();
}

String EndDevice::getPartId() const
{
    return m_SerialNum;
}

/* */
SSI_Status EndDevice::getInfo(SSI_EndDeviceInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->endDeviceHandle = getHandle();
    getId().get(pInfo->uniqueId, sizeof(pInfo->uniqueId));
    getAddress(pInfo->endDeviceAddress);
    pInfo->deviceType = getDeviceType();

    Controller *pController = getController();
    if (pController != NULL) {
        pInfo->controllerHandle = pController->getHandle();
    } else {
        pInfo->controllerHandle = SSI_NULL_HANDLE;
    }

    RaidInfo *pRaidInfo = getRaidInfo();
    if (pRaidInfo != NULL) {
        pInfo->raidInfoHandle = pRaidInfo->getHandle();
    } else {
        pInfo->raidInfoHandle = SSI_NULL_HANDLE;
    }

    pInfo->storagePool = getStoragePoolId();
    Array *pArray = getArray();
    if (pArray != NULL) {
        pInfo->arrayHandle = pArray->getHandle();
    } else {
        pInfo->arrayHandle = SSI_NULL_HANDLE;
    }

    Enclosure *pEnclosure = getEnclosure();
    if (pEnclosure != NULL) {
        pInfo->enclosureHandle = pEnclosure->getHandle();
        pEnclosure->getSlotAddress(pInfo->slotAddress, getSlotNumber());
    } else {
        pInfo->enclosureHandle = SSI_NULL_HANDLE;
        pInfo->slotAddress.scsiAddress.host = 0;
        pInfo->slotAddress.scsiAddress.bus = 0;
        pInfo->slotAddress.scsiAddress.target = 0;
        pInfo->slotAddress.scsiAddress.lun = 0;
        pInfo->slotAddress.sasAddressPresent = SSI_FALSE;
        pInfo->slotAddress.sasAddress = 0;
    }

    m_VendorId.get(pInfo->vendorId, sizeof(pInfo->vendorId));
    m_SerialNum.get(pInfo->serialNo, sizeof(pInfo->serialNo));
    m_Model.get(pInfo->model, sizeof(pInfo->model));
    m_Firmware.get(pInfo->firmware, sizeof(pInfo->firmware));

    pInfo->diskType = getDiskType();
    pInfo->state = getDiskState();
    pInfo->usage = getDiskUsage();
    pInfo->totalSize = m_TotalSize;
    pInfo->logicalSize = m_LogicalSectorSize;
    pInfo->physicalSize = m_PhysicalSectorSize;
    pInfo->blocksTotal = m_BlocksTotal;
    pInfo->freeSize = m_BlocksFree * m_LogicalSectorSize;

    pInfo->writeCachePolicy = m_WriteCachePolicy;
    pInfo->systemDisk = isSystemDisk() ? SSI_TRUE : SSI_FALSE;
    pInfo->slotNumber = getSlotNumber();
    pInfo->locateLEDSupport = SSI_TRUE;     /* Like in Windows SSI. If disk is connected to Intel controller it should be true and all detected controllers are Intel's. */
    pInfo->isPreBootVisible = pEnclosure ? SSI_FALSE : SSI_TRUE;
    pInfo->ledState = m_ledState;
    pInfo->systemIoBusNumber = m_systemIoBusNumer;
    pInfo->PCISlotNumber = m_PCISlotNumber;

    /* FDx8 */
    pInfo->Isx8A = SSI_FALSE;
    pInfo->Isx8B = SSI_FALSE;
    if (m_FDx8Disk == 1) {
        pInfo->Isx8A = SSI_TRUE;
    } else if (m_FDx8Disk == 2) {
        pInfo->Isx8B = SSI_TRUE;
    }

    /* VMD domain */
    pInfo->vmdDomain = getVmdDomain();

    pInfo->IsIntelNVMe = m_isIntelNvme ? SSI_TRUE : SSI_FALSE;

    return SSI_StatusOk;
}

/* */
SSI_Status EndDevice::locate(bool mode) const
{
    String tmp = mode?"locate":"normal";
    if (shell_command("ledctl " + tmp + "='/dev/" + m_DevName + "'") == 0) {
        return SSI_StatusOk;
    } else {
        return SSI_StatusFailed;
    }
}

/* */
RaidInfo * EndDevice::getRaidInfo() const
{
    return m_pPort ? m_pPort->getRaidInfo() : NULL;
}

unsigned int EndDevice::getSlotNumber() const
{
    if (m_pEnclosure == NULL) {
        return -1U;
    }

    return m_pEnclosure->getSlotNumber(m_SASAddress);
}

/* */
void EndDevice::getAddress(SSI_Address &address) const
{
    address.scsiAddress = m_SCSIAddress;
    address.sasAddress = m_SASAddress;
    address.bdfAddress = m_BDFAddress;
    address.sasAddressPresent = m_SASAddress ? SSI_TRUE : SSI_FALSE;
    address.bdfAddressPresent = m_BDFAddress.bus != 0 || m_BDFAddress.device != 0;
}


/* */
void EndDevice::getPhys(Container<Phy> &container) const
{
    container.clear();
    container.add(m_pPhy);
}

/* */
void EndDevice::getPorts(Container<Port> &container) const
{
    container.clear();
    container.add(m_pPort);
}

/* */
SSI_Status EndDevice::passthroughCmd(void *, void *, unsigned int, SSI_DataDirection)
{
    return SSI_StatusNotSupported;
}

/* */
void EndDevice::addToSession(Session *pSession)
{
    pSession->addEndDevice(this);
    pSession->addPhy(m_pPhy);
    pSession->addPort(m_pPort);
}

/* */
bool EndDevice::operator ==(const Object &object) const
{
    return typeid(*this) == typeid(object) &&
        static_cast<const EndDevice *>(&object)->getSerialNum() == m_SerialNum;
}

/* */
void EndDevice::attachPhy(Phy *pPhy)
{
    m_pPhy = pPhy;
}

/* */
void EndDevice::attachPort(Port *pPort)
{
    m_pPort = pPort;
}

void EndDevice::determineBlocksFree(Array *pArray)
{
    uint64_t raidSectorSize = DEFAULT_SECTOR_SIZE;
    unsigned long long int totalBlocks = -1ULL;
    unsigned long long occupiedBlocks = 0;
    unsigned int stripSize = 0;
    int volumeCount = 0;

    Container<Volume> volumes;
    pArray->getVolumes(volumes);

    if (!volumes.empty()) {
        raidSectorSize = volumes.front()->getLogicalSectorSize();
    }
    foreach (volume, volumes) {
        Container<EndDevice> endDevices;
        (*volume)->getEndDevices(endDevices, true);
        foreach (endDevice, endDevices) {
            if ((*endDevice)->getSerialNum() == m_SerialNum) {
                occupiedBlocks += ((unsigned long long) (*volume)->getComponentSize() * KILOBYTE) / raidSectorSize;
                occupiedBlocks += IMSM_RESERVED_SECTORS;
                stripSize = (*volume)->getStripSize();
                volumeCount++;
                foreach (endDevice2, endDevices) {
                    totalBlocks = ssi_min(totalBlocks, (*endDevice2)->getTotalSize() / raidSectorSize);
                }
                break;
            }
        }
    }

    if (volumeCount == 0) {
        m_BlocksFree = m_BlocksTotal;
    } else {
        if (occupiedBlocks > 0) {
            occupiedBlocks += MPB_SECTOR_CNT;
        }
        if (occupiedBlocks > totalBlocks) {
            m_BlocksFree = 0;
        } else {
            m_BlocksFree = totalBlocks - occupiedBlocks;
        }
    }

    if ((m_BlocksFree < stripSize / raidSectorSize) || (volumeCount > 1)) {
        m_BlocksFree = 0;
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
