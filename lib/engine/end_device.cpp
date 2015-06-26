
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

#include <scsi/sg_lib.h>
#include <scsi/sg_cmds_basic.h>

#include <ssi.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "storage_device.h"
#include "raid_device.h"
#include "array.h"
#include "volume.h"
#include "end_device.h"
#include "phy.h"
#include "raid_info.h"
#include "port.h"
#include "remote_port.h"
#include "session.h"
#include "enclosure.h"
#include "utils.h"

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
EndDevice::EndDevice(const String &path)
    : StorageDevice(path),
      m_SerialNum(""),
      m_pPhy(NULL),
      m_pPort(NULL),
      m_pEnclosure(NULL),
      m_Model(""),
      m_Firmware(""),
      m_TotalSize(0),
      m_BlockSize(0),
      m_BlocksFree(0),
      m_LogicalSectorSize(0),
      m_PhysicalSectorSize(0),
      m_SASAddress(0),
      m_WriteCachePolicy(SSI_WriteCachePolicyOff),
      m_ledState(0),
      m_systemIoBusNumer(0),
      m_PCISlotNumber(0)
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
        SysfsAttr attr =  "/sys/class/block/" + m_DevName + "/size";
        attr >> m_BlockSize;
        m_TotalSize = (unsigned long long) m_BlockSize * m_LogicalSectorSize;
    } catch (...) {
    }

    getAtaDiskInfo("/dev/"+ m_DevName, m_Model, m_SerialNum, m_Firmware);
    m_SerialNum.trim();
    m_Model.trim();
    m_Firmware.trim();

    unsigned char buffer[4096];
    bool hdioNotSupported = true;

    int fd = open("/dev/" + m_DevName, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        struct hd_driveid id;
        if (ioctl(fd, HDIO_GET_IDENTITY, &id) >= 0) {
        if (m_SerialNum.isEmpty()) {
        m_SerialNum.assign(reinterpret_cast<char *>(id.serial_no), sizeof(id.serial_no));
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
            if (m_SerialNum.isEmpty() && shell_cap("sg_inq /dev/" + m_DevName, sbuffer) == 0) {
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

#define ATTR_PACKED __attribute__((packed))

#pragma pack(1)
struct ata_identify_device {
    unsigned short words000_009[10];
    char  serial_no[20];
    unsigned short words020_022[3];
    char  fw_rev[8];
    char  model[40];
    unsigned short words047_079[33];
    unsigned short major_rev_num;
    unsigned short minor_rev_num;
    unsigned short command_set_1;
    unsigned short command_set_2;
    unsigned short command_set_extension;
    unsigned short cfs_enable_1;
    unsigned short word086;
    unsigned short csf_default;
    unsigned short words088_255[168];
} ATTR_PACKED;
#pragma pack()

void EndDevice::copy2le(char *dest, const char *src, size_t n)
{
#ifdef WORDS_BIGENDIAN
    memcpy(dest, src, n);
#else
  for (size_t i = 0; i < n; i += 2) {
    dest[i]   = src[i+1];
    dest[i+1] = src[i];
  }
#endif
}

/**
 * @brief Fills model, vendor and firmware properties for ATA drives using sg_sat_identify
 *
 * @return  0 for success, -1 if popen fails and status of sg_sat_identify otherwise
 */
int EndDevice::getAtaDiskInfo(const String &devName, String &model, String &serial, String &firmware)
{
    int status;
    ata_identify_device ata;
    size_t size = sizeof(ata);
    /* one more to hold trailing zero */
    char buf[sizeof(ata.model) + 1];
    status = shell_cap("sg_sat_identify --raw " + devName + " 2>/dev/null", &ata, size);
    if (status != 0)
    return status;
    if (size != sizeof(ata))
    return -1;
    buf[sizeof(ata.serial_no)] = '\0';
    copy2le(buf, const_cast<const char *>(ata.serial_no), sizeof(ata.serial_no));
    serial.assign(buf);
    buf[sizeof(ata.model)] = '\0';
    copy2le(buf, const_cast<const char *>(ata.model), sizeof(ata.model));
    model.assign(buf);
    buf[sizeof(ata.fw_rev)] = '\0';
    copy2le(buf, const_cast<const char *>(ata.fw_rev), sizeof(ata.fw_rev));
    firmware.assign(buf);
    return 0;
}

/* */
EndDevice::~EndDevice()
{
}

/* */
SSI_Status EndDevice::getInfo(SSI_EndDeviceInfo *pInfo) const
{
    if (pInfo == NULL) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->endDeviceHandle = getId();
    getAddress(pInfo->endDeviceAddress);
    pInfo->deviceType = getDeviceType();

    RaidInfo *pRaidInfo = getRaidInfo();
    if (pRaidInfo != NULL) {
        pInfo->raidInfoHandle = pRaidInfo->getId();
    } else {
        pInfo->raidInfoHandle = SSI_NULL_HANDLE;
    }
    pInfo->storagePool = getStoragePoolId();
    Array *pArray = getArray();
    if (pArray != NULL) {
        pInfo->arrayHandle = pArray->getId();
    } else {
        pInfo->arrayHandle = SSI_NULL_HANDLE;
    }
    Enclosure *pEnclosure = getEnclosure();
    if (pEnclosure != NULL) {
        pInfo->enclosureHandle = pEnclosure->getId();
        pEnclosure->getSlotAddress(pInfo->slotAddress,
            getSlotNumber());
    } else {
        pInfo->enclosureHandle = SSI_NULL_HANDLE;
        pInfo->slotAddress.scsiAddress.host = 0;
        pInfo->slotAddress.scsiAddress.bus = 0;
        pInfo->slotAddress.scsiAddress.target = 0;
        pInfo->slotAddress.scsiAddress.lun = 0;
        pInfo->slotAddress.sasAddressPresent = SSI_FALSE;
        pInfo->slotAddress.sasAddress = 0;
    }

    m_SerialNum.get(pInfo->serialNo, sizeof(pInfo->serialNo));
    m_Model.get(pInfo->model, sizeof(pInfo->model));
    m_Firmware.get(pInfo->firmware, sizeof(pInfo->firmware));

    pInfo->diskType = getDiskType();
    pInfo->state = getDiskState();
    pInfo->usage = getDiskUsage();
    pInfo->totalSize = m_TotalSize;
    pInfo->blockSize = m_BlockSize;
    pInfo->blocksTotal = (m_BlockSize * 512);
    pInfo->blocksFree = m_BlocksFree;

    pInfo->writeCachePolicy = m_WriteCachePolicy;
    if (isSystemDisk()) {
        pInfo->systemDisk = SSI_TRUE;
    } else {
        pInfo->systemDisk = SSI_FALSE;
    }
    pInfo->slotNumber = getSlotNumber();
    pInfo->locateLEDSupport = SSI_FALSE;
    pInfo->isPreBootVisible = pEnclosure?SSI_FALSE:SSI_TRUE;
    pInfo->ledState = m_ledState;
    pInfo->systemIoBusNumber = m_systemIoBusNumer;
    pInfo->PCISlotNumber = m_PCISlotNumber;

    return SSI_StatusOk;
}

SSI_Status EndDevice::locate(bool mode)
{
    String tmp = mode?"locate":"normal";
    if (shell("ledctl " + tmp + "=/dev/" + m_DevName) == 0)
        return SSI_StatusOk;
    else
        return SSI_StatusFailed;
}


/* */
RaidInfo * EndDevice::getRaidInfo() const
{
    return m_pPort ? m_pPort->getRaidInfo() : NULL;
}

unsigned int EndDevice::getSlotNumber() const
{
    if (m_pEnclosure == NULL)
        return -1U;
    return m_pEnclosure->getSlotNumber(m_SASAddress);
}

/* */
void EndDevice::getAddress(SSI_Address &address) const
{
    address.scsiAddress = m_SCSIAddress;
    address.sasAddress = m_SASAddress;
    address.sasAddressPresent = m_SASAddress ? SSI_TRUE : SSI_FALSE;
}


/* */
void EndDevice::getPhys(Container<Phy> &container) const
{
    container.clear();
    container.add(m_pPhy);
}

/* */
SSI_Status EndDevice::passthroughCmd(void *pInfo, void *pData, unsigned int dataSize,
    SSI_DataDirection dir)
{
    (void)pInfo;
    (void)pData;
    (void)dataSize;
    (void)dir;
    return SSI_StatusOk;
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
String EndDevice::getKey() const
{
    return m_SerialNum;
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

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
