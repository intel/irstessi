/*
Copyright (c) 2011 - 2017, Intel Corporation
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
#include <limits>
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
using std::numeric_limits;
using boost::shared_ptr;

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

/* */
#define ATA_CMD_ID_ATA 0xEC

struct AtaCommand
{
    __uint8_t command;
    __uint8_t obsolete1;
    __uint8_t obsolete2;
    __uint8_t transportDependent;
};

struct AtaIdentifyCall
{
    AtaCommand command;
    __uint16_t data[256];
};

namespace {
    __uint16_t swap(__uint16_t value)
    {
        return (value >> 8) | (value << 8);
    }
} /* <<anonymous>> */

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
      m_pPhy(),
      m_pPort(),
      m_pEnclosure(),
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
      m_vmdDomain(numeric_limits<unsigned int>::max()), /* -1 by default because 0 is a valid value */
      m_isIntelNvme(false),
      m_locateLedSupport(true) /* Like in Windows SSI - if disk is connected to Intel controller, it should be true.
                                  In Linux SSI, all detected controllers are Intel's. */
{
    m_SCSIAddress.bus = 0;
    m_SCSIAddress.host = 0;
    m_SCSIAddress.lun = 0;
    m_SCSIAddress.target = 0;
    m_BDFAddress.bus = 0;
    m_BDFAddress.device = 0;
    m_BDFAddress.domain = 0;
    m_BDFAddress.function = 0;
}

/* */
void EndDevice::discover()
{
    m_pPhy = shared_ptr<Phy>(new Phy(m_Path, 0, shared_from_this()));
    m_pPort = shared_ptr<Port>(new RemotePort(m_Path));
    m_pPort->setParent(shared_from_this());
    m_pPort->attachPhy(m_pPhy);

    String scsiAddress = m_Path.reverse_after("/");

    m_SCSIAddress.host = scsiAddress;
    scsiAddress = scsiAddress.after(":");
    m_SCSIAddress.bus = scsiAddress;
    scsiAddress = scsiAddress.after(":");
    m_SCSIAddress.target = scsiAddress;
    scsiAddress = scsiAddress.after(":");
    m_SCSIAddress.lun = scsiAddress;

    /* BDF address is currently used only for NVMe/VMD */
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

int EndDevice::getAtaDiskInfo(const String &devPath, String &model, String &serial, String &firmware)
{
    int fd = open(devPath, O_RDONLY | O_DIRECT | O_NONBLOCK);

    if (fd < 0) {
        return -1;
    }

    AtaIdentifyCall call = {};
    call.command.command = ATA_CMD_ID_ATA;
    call.command.transportDependent = 1;

    int ret = ioctl(fd, HDIO_DRIVE_CMD, &call);
    if (ret < 0) {
        close(fd);
        return -1;
    }

    for (size_t i = 0; i < sizeof(call.data) / sizeof(call.data[0]); ++i) {
        call.data[i] = swap(call.data[i]);
    }

    /* Values taken from ATA/ATAPI Command Set - 2 (ACS-2), chapter 7.18.7 */
    const size_t SerialFirstWord = 10;
    const size_t SerialLength = 10;
    const size_t FirmwareFirstWord = 23;
    const size_t FirmwareLength = 4;
    const size_t ModelFirstWord = 27;
    const size_t ModelLength = 20;

    serial = String(reinterpret_cast<char*>(call.data + SerialFirstWord), SerialLength * sizeof(call.data[0]));
    firmware = String(reinterpret_cast<char*>(call.data + FirmwareFirstWord), FirmwareLength * sizeof(call.data[0]));
    model = String(reinterpret_cast<char*>(call.data + ModelFirstWord), ModelLength * sizeof(call.data[0]));

    serial.trim();
    firmware.trim();
    model.trim();

    close(fd);
    return 0;
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

    shared_ptr<Controller> pController = getController();
    if (pController) {
        pInfo->controllerHandle = pController->getHandle();
    } else {
        pInfo->controllerHandle = SSI_NULL_HANDLE;
    }

    shared_ptr<RaidInfo> pRaidInfo = getRaidInfo();
    if (pRaidInfo) {
        pInfo->raidInfoHandle = pRaidInfo->getHandle();
    } else {
        pInfo->raidInfoHandle = SSI_NULL_HANDLE;
    }

    pInfo->storagePool = getStoragePoolId();
    shared_ptr<Array> pArray = getArray();
    if (pArray) {
        pInfo->arrayHandle = pArray->getHandle();
    } else {
        pInfo->arrayHandle = SSI_NULL_HANDLE;
    }

    shared_ptr<Enclosure> pEnclosure = getEnclosure();
    if (pEnclosure) {
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
    pInfo->physicalSlotNumber = getSlotNumber();
    pInfo->locateLEDSupport = m_locateLedSupport ? SSI_TRUE : SSI_FALSE;
    pInfo->removeDiskSupport = canRemoveDisk() ? SSI_TRUE : SSI_FALSE;
    pInfo->isPreBootVisible = pEnclosure ? SSI_FALSE : SSI_TRUE;
    pInfo->ledState = m_ledState;
    pInfo->systemIoBusNumber = m_systemIoBusNumer;
    pInfo->PCISlotNumber = m_PCISlotNumber;

    /* FDx8 */
    pInfo->isx8A = SSI_FALSE;
    pInfo->isx8B = SSI_FALSE;
    if (m_FDx8Disk == 1) {
        pInfo->isx8A = SSI_TRUE;
    } else if (m_FDx8Disk == 2) {
        pInfo->isx8B = SSI_TRUE;
    }

    /* VMD domain */
    pInfo->vmdDomainIndex = getVmdDomain();

    pInfo->isIntelNVMe = m_isIntelNvme ? SSI_TRUE : SSI_FALSE;

    /* RSTe Windows Only */
    pInfo->socketNumber = 0;
    pInfo->vmdControllerNumber = 0;
    pInfo->rootPortOffset = 0;

    return SSI_StatusOk;
}

/* */
SSI_Status EndDevice::locate(bool mode) const
{
    String tmp = mode ? "locate" : "normal";
    if (shell_command("ledctl " + tmp + "='/dev/" + m_DevName + "'") == 0) {
        return SSI_StatusOk;
    } else {
        return SSI_StatusFailed;
    }
}

/* */
SSI_Status EndDevice::removeDisk()
{
    if (!canRemoveDisk()) {
        return SSI_StatusNotSupported;
    }

    if (SSI_Status status = locate(true)) {
        return status;
    }

    return removeDevice();
}

/* */
shared_ptr<RaidInfo> EndDevice::getRaidInfo() const
{
    return m_pPort ? m_pPort->getRaidInfo() : shared_ptr<RaidInfo>();
}

unsigned long long EndDevice::getSlotNumber() const
{
    if (!m_pEnclosure) {
        return -1ULL;
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
void EndDevice::addToSession(const shared_ptr<Session>& pSession)
{
    pSession->addEndDevice(shared_from_this());
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
void EndDevice::attachPhy(const shared_ptr<Phy>& pPhy)
{
    m_pPhy = pPhy;
}

/* */
void EndDevice::attachPort(const shared_ptr<Port>& pPort)
{
    m_pPort = pPort;
}

/* */
void EndDevice::determineBlocksFree(const shared_ptr<Array>& pArray)
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
        if (raidSectorSize == 0) {
            raidSectorSize = DEFAULT_SECTOR_SIZE;
        }
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
