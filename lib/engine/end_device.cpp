
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

#include <scsi/sg_lib.h>
#include <scsi/sg_cmds_basic.h>

#include <ssi.h>

#include "exception.h"
#include "list.h"
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
      m_pPhy(0),
      m_pPort(0),
      m_pEnclosure(0),
      m_Model(""),
      m_Firmware(""),
      m_TotalSize(0),
      m_BlockSize(0),
      m_BlocksFree(0),
      m_SASAddress(0),
      m_WriteCachePolicy(SSI_WriteCachePolicyOff)
{
    m_pPhy = new Phy(path, 0);
    m_pPhy->setParent(this);
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
    for (Iterator<Directory *> i = dir; *i != 0; ++i) {
        temp = *(*i) + "device";
        if (temp == m_Path) {
            m_DevName = (*i)->reverse_after("/");
            break;
        }
    }
    temp = m_Path + "/generic";
    m_SgName = temp.reverse_after("/");

    unsigned char buffer[4096];
    bool hdioNotSupported = true, totalSizeNotSupported = false;

    int fd = open("/dev/" + m_DevName, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        struct hd_driveid id;
        if (ioctl(fd, HDIO_GET_IDENTITY, &id) >= 0) {
            m_SerialNum.assign(reinterpret_cast<char *>(id.serial_no), sizeof(id.serial_no));
            m_SerialNum.trim();
            m_Model.assign(reinterpret_cast<char *>(id.model), sizeof(id.model));
            m_Model.trim();
            m_Firmware.assign(reinterpret_cast<char *>(id.fw_rev), sizeof(id.fw_rev));
            m_Firmware.trim();
            m_BlockSize = id.lba_capacity_2;
            if ((id.command_set_1 & id.cfs_enable_1) != 0) {
                m_WriteCachePolicy = SSI_WriteCachePolicyOn;
            }
            hdioNotSupported = false;
        }
#ifdef BLKGETSIZE64
        if (ioctl(fd, BLKGETSIZE64, &m_TotalSize) < 0) {
#endif /* BLKGETSIZE64 */
            unsigned int totalSize;
            if (ioctl(fd, BLKGETSIZE, &totalSize) < 0) {
                totalSizeNotSupported = true;
            } else {
                m_TotalSize = totalSize; m_TotalSize *= 512;
            }
#ifdef BLKGETSIZE64
        }
#endif /* BLKGETSIZE64 */
        close(fd);
    }
    fd = sg_cmds_open_device("/dev/" + m_SgName, 0, 0);
    if (fd >= 0) {
        if (hdioNotSupported) {
            if (sg_ll_inquiry(fd, 0, 0, 0, buffer, sizeof(buffer), 1, 0) == 0) {
//              m_SerialNum.assign(reinterpret_cast<char *>((buffer) + SCSI_INQUIRY_SERIALNUM_OFFSET), HD_SERIALNO_LENGTH);
//              m_SerialNum.trim();
                m_Model.assign(reinterpret_cast<char *>(buffer) + SCSI_INQUIRY_MODEL_OFFSET, SSI_PRODUCT_ID_LENGTH);
                m_Model.trim();
                m_Firmware.assign(reinterpret_cast<char *>(buffer) + SCSI_INQUIRY_FW_OFFSET, SSI_PRODUCT_REV_LENGTH);
                m_Firmware.trim();
            }
            String sbuffer;
            if (shell_cap("sg_inq /dev/" + m_DevName, sbuffer) == 0) {
                m_SerialNum = sbuffer.between("Unit serial number: ", "\n");
                m_SerialNum.trim();
            }
            if (sg_ll_mode_sense10(fd, 0, 0, 0, SCSI_SENSE_DISK_SETTINGS_PAGE, 0, buffer, sizeof(buffer), 1, 0) == 0) {
                if ((*(buffer + SCSI_SENSE_DISK_CACHE_OFFSET) & SCSI_SENSE_DISK_CACHE_MASK) != 0) {
                    m_WriteCachePolicy = SSI_WriteCachePolicyOn;
                }
            }
        }
        if (totalSizeNotSupported) {
            if (sg_ll_readcap_10(fd, 0, 0, buffer, sizeof(buffer), 1, 0) == 0) {
                m_BlockSize = sg_make_int(buffer);
                if (m_BlockSize == UINT_MAX) {
                    m_BlockSize = sg_make_int(buffer + 4);
                }
                m_BlockSize++;
            }
        }
        sg_cmds_close_device(fd);
    }
}

/* */
EndDevice::~EndDevice()
{
}

/* */
SSI_Status EndDevice::getInfo(SSI_EndDeviceInfo *pInfo) const
{
    if (pInfo == 0) {
        return SSI_StatusInvalidParameter;
    }
    pInfo->endDeviceHandle = getId();
    getAddress(pInfo->endDeviceAddress);
    pInfo->deviceType = getDeviceType();

    RaidInfo *pRaidInfo = getRaidInfo();
    if (pRaidInfo != 0) {
        pInfo->raidInfoHandle = pRaidInfo->getId();
    } else {
        pInfo->raidInfoHandle = SSI_NULL_HANDLE;
    }
    pInfo->storagePool = getStoragePoolId();
    Array *pArray = getArray();
    if (pArray != 0) {
        pInfo->arrayHandle = pArray->getId();
    } else {
        pInfo->arrayHandle = SSI_NULL_HANDLE;
    }
    Enclosure *pEnclosure = getEnclosure();
    if (pEnclosure != 0) {
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

    return SSI_StatusOk;
}

/* */
RaidInfo * EndDevice::getRaidInfo() const
{
    return m_pPort ? m_pPort->getRaidInfo() : 0;
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
    container = m_pPhy;
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
void EndDevice::acquireId(Session *pSession)
{
    pSession->addEndDevice(this);
    pSession->addPhy(m_pPhy);
    pSession->addPort(m_pPort);
}

/* */
bool EndDevice::equal(const Object *pObject) const
{
    return Object::equal(pObject) &&
        dynamic_cast<const EndDevice *>(pObject)->getSerialNum() == m_SerialNum;
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
