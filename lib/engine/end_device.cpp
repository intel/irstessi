/*
 * IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING.
 * BY LOADING OR USING THIS SOFTWARE, YOU AGREE TO THE TERMS OF THIS INTEL
 * SOFTWARE LICENSE AGREEMENT.  IF YOU DO NOT WISH TO SO AGREE, DO NOT COPY,
 * INSTALL OR USE THIS SOFTWARE.
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
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
#define SCSI_INQUIRY_SERIALNUM_OFFSET   4

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
    : StorageDevice(0, path),
      m_SerialNum(""),
      m_pPhy(0),
      m_pPort(0),
      m_Model(""),
      m_Firmware(""),
      m_TotalSize(0),
      m_BlockSize(0),
      m_BlocksFree(0),
      m_SASAddress(0),
      m_WriteCachePolicy(SSI_WriteCachePolicyOff)
{
    m_pPhy = new Phy(this, path, 0);
    m_pPort = new RemotePort(this, path);
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
                m_SerialNum.assign(reinterpret_cast<char *>(buffer) + SCSI_INQUIRY_SERIALNUM_OFFSET, HD_SERIALNO_LENGTH);
                m_SerialNum.trim();
                m_Model.assign(reinterpret_cast<char *>(buffer) + SCSI_INQUIRY_MODEL_OFFSET, SSI_PRODUCT_ID_LENGTH);
                m_Model.trim();
                m_Firmware.assign(reinterpret_cast<char *>(buffer) + SCSI_INQUIRY_FW_OFFSET, SSI_PRODUCT_REV_LENGTH);
                m_Firmware.trim();
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
void EndDevice::getPhys(Container &container) const
{
    container.clear();
    if (m_pPhy != 0) {
        container.add(reinterpret_cast<Object *>(m_pPhy));
    }
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
void EndDevice::attachPhy(Object *pPhy)
{
    m_pPhy = dynamic_cast<Phy *>(pPhy);
}

/* */
void EndDevice::attachPort(Object *pPort)
{
    m_pPort = dynamic_cast<Port *>(pPort);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
