
/*
Copyright (c) 2011 - 2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/**
 * @file    SSI\ssi.h
 *
 * @brief   This file specifies the Standard Storage Interface(SSI).
**/

#ifndef __SSI_H
#define __SSI_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER

#ifdef ISDI_2_EXPORTS
#define SSI_API __declspec(dllexport)
#else
#define SSI_API __declspec(dllimport)
#endif

#else
#define SSI_API
#pragma GCC visibility push(default)
#endif

/**
 * @name Base Types
 * These typedefs defines the base types used in this specification
*/
/*@{*/
typedef char                SSI_Char;
typedef unsigned char       SSI_Uint8;
typedef char                SSI_Int8;
typedef unsigned short      SSI_Uint16;
typedef short               SSI_Int16;
typedef unsigned int        SSI_Uint32;
typedef int                 SSI_Int32;
typedef long long           SSI_Int64;
typedef unsigned long long  SSI_Uint64;
/*@}*/

/**
 * @typedef SSI_Bool
 * @brief This defines the Boolean type for SSI.
*/
typedef SSI_Uint32 SSI_Bool;

/**
 * @name Boolean Defines
 * These typedefs defines the SSI_TRUE & SSI_FALSE for the specification
*/
/*@{*/
#define SSI_TRUE 1
#define SSI_FALSE 0
/*@}*/

/**
 * @typedef SSI_Handle
 * @brief This defines the SSI_Handle which is used as opaque reference to objects.
*/
typedef SSI_Uint32 SSI_Handle;

/**
 * @var SSI_NULL_HANDLE
 * @brief Defines the Null Handle reprensented in the specification.
*/
static const SSI_Handle SSI_NULL_HANDLE = 0;

/**
 * @var SSI_INVALID_SLOTNUMBER
 * @brief Defines the Invalid SlotNumber reprensented in the specification.
*/
static const SSI_Uint32 SSI_INVALID_SLOTNUMBER = 0xFFFFFFFF;

/**
 * @def SSI_LIBRARY_VERSION
 *
 * @brief   Version of ssi api library
**/
#define SSI_LIBRARY_VERSION "0.1.0.1"

/**
 * @def SSI_VOLUME_NAME_LENGTH
 *
 * @brief   Ssi volume name length.
**/
#define SSI_VOLUME_NAME_LENGTH 17

/**
 * @def SSI_SOFTWARE_VERSION_LENGTH
 *
 * @brief   Ssi software version length.
**/
#define SSI_SOFTWARE_VERSION_LENGTH 32

/**
 * @def SSI_VENDOR_LENGTH
 *
 * @brief   Ssi vendor length.
**/
#define SSI_VENDOR_LENGTH 8

/**
 * @def SSI_PRODUCT_ID_LENGTH
 *
 * @brief   Ssi product identifier length.
**/
#define SSI_PRODUCT_ID_LENGTH 16

/**
 * @def SSI_PRODUCT_REV_LENGTH
 *
 * @brief   Ssi product reverse length.
**/
#define SSI_PRODUCT_REV_LENGTH 5

/**
 * @def SSI_PRODUCT_REV_LENGTH
 *
 * @brief   Ssi logical id length.
**/
#define SSI_LOGICAL_ID_LENGTH 17

/**
 * @def SSI_CONTROLLER_NAME_LENGTH
 *
 * @brief   Ssi controller name length.
**/
#define SSI_CONTROLLER_NAME_LENGTH 256

/**
 * @def SSI_END_DEVICE_SERIALNO_LENGTH
 *
 * @brief   Ssi end device serialno length.
**/
#define SSI_END_DEVICE_SERIALNO_LENGTH 21

/**
 * @def SSI_END_DEVICE_MODEL_LENGTH
 *
 * @brief   Ssi end device model length.
**/
#define SSI_END_DEVICE_MODEL_LENGTH 41

/**
 * @def SSI_END_DEVICE_FIRMWARE_LENGTH
 *
 * @brief   Ssi end device firmware length.
**/
#define SSI_END_DEVICE_FIRMWARE_LENGTH 21

/**
 * @def SSI_ROUTING_DEVICE_COMPONENT_LENGTH
 *
 * @brief   Ssi routing device component length.
**/
#define SSI_ROUTING_DEVICE_COMPONENT_LENGTH 8

/**
 * @def SSI_ROUTING_DEVICE_COMPONENT_REV_LENGTH
 *
 * @brief   Ssi routing device component reverse length.
**/
#define SSI_ROUTING_DEVICE_COMPONENT_REV_LENGTH 5

/**
 * @def SSI_ARRAY_NAME_LENGTH
 *
 * @brief   Ssi array name length.
**/
#define SSI_ARRAY_NAME_LENGTH 21

/**
 * @def SSI_DISK_PASSWORD_LENGTH
 *
 * @brief   Ssi disk password length.
**/
#define SSI_DISK_PASSWORD_LENGTH 32

/**
 * @def SSI_DEFAULT_STORAGE_POOL
 *
 * @brief   Ssi default storage pool.
**/
#define SSI_DEFAULT_STORAGE_POOL 0

/**
 * @enum    SSI_Status
 *
 * @brief   Values that represent SSI_Status.
**/
typedef enum _SSI_Status
{
    /** Function completed successfully */
    SSI_StatusOk                     =  0,
    /** Library was unable to allocate a new session */
    SSI_StatusInsufficientResources  =  1,
    /** Null pointer was passed to a out parameter */
    SSI_StatusInvalidParameter       =  2,
    /** Invalid handle was passed to a function */
    SSI_StatusInvalidHandle          =  3,
    /** The stripe size is not valid for the specific RAID level */
    SSI_StatusInvalidStripSize       =  4,
    /** (Volume name) parameter contains one or more invalid characters */
    SSI_StatusInvalidString          =  5,
    /** Invalid volume size parameter */
    SSI_StatusInvalidSize            =  6,
    /** Invalid RAID level parameter for volume creation / migration */
    SSI_StatusInvalidRaidLevel       =  7,
    /** Session handle is invalid (previously destroyed) */
    SSI_StatusInvalidSession         =  8,
    /** Call failed to complete within specified timeframe */
    SSI_StatusTimeout                =  9,
    /** Function is not supported on this platform */
    SSI_StatusNotImplemented         = 10,
    /** Library has already been initialized */
    SSI_StatusDuplicate              = 11,
    /** Library hasn't been initialized, yet */
    SSI_StatusNotInitialized         = 12,
    /** Not enough items (disks, etc) have been specified */
    SSI_StatusBufferTooSmall         = 13,
    /** Action is not supported */
    SSI_StatusNotSupported           = 14,
    /** Function failed to complete */
    SSI_StatusFailed                 = 15,
    /** Finalize failed to cleanup library */
    SSI_StatusInternalError          = 16,
    /** Scope passed is either invalid or not valid for current operation */
    SSI_StatusInvalidScope           = 17,
    /** Item not in appropriate state for action to complete successfully */
    SSI_StatusInvalidState           = 18,
    /** Maximum number of unlock attempts has been exceeded */
    SSI_StatusRetriesExceeded        = 19,
    /** Password is incorrect for locked item */
    SSI_StatusWrongPassword          = 20,
    /** Invalid meta data size */
    SSI_StatusDataExceedsLimits      = 21,
    /** User has insufficient privileges to execute function */
    SSI_StatusInsufficientPrivileges = 22,
    /** Driver has returned a busy notification */
    SSI_StatusDriverBusy             = 23,
    /** Too many items (disks, etc) have been specified */
    SSI_StatusBufferTooLarge         = 24
} SSI_Status;

/**
 * @enum    SSI_VolumeCachePolicy
 *
 * @brief   Values that represent SSI_VolumeCachePolicy.
**/
typedef enum _SSI_VolumeCachePolicy
{
    /** Cache policy is unknown */
    SSI_VolumeCachePolicyUnknown      = 0,
    /** Cache is disabled */
    SSI_VolumeCachePolicyOff          = 1,
    /** Write operations are not cached */
    SSI_VolumeCachePolicyWriteThrough = 2,
    /** Write operations are cached */
    SSI_VolumeCachePolicyWriteBack    = 3
} SSI_VolumeCachePolicy;

/**
 * @enum    SSI_WriteCachePolicy
 *
 * @brief   Values that represent SSI_WriteCachePolicy.
**/
typedef enum _SSI_WriteCachePolicy
{
    /** Cache policy is unknown */
    SSI_WriteCachePolicyUnknown = 0,
    /** Cache is disabled */
    SSI_WriteCachePolicyOff     = 1,
    /** Write cache is enabled */
    SSI_WriteCachePolicyOn      = 2
} SSI_WriteCachePolicy;

/**
 * @enum    SSI_VolCacheSize
 *
 * @brief   Values that represent SSI_VolCacheSize.
**/
typedef enum _SSI_VolCacheSize
{
    /** Cache size is unknown */
    SSI_VolCacheSizeUnknown  = 0x00,
    /** Cache is in safe mode (write caching disabled) */
    SSI_VolCacheSizeSafeMode = 0x01,
    /** Inbox driver settings */
    SSI_VolCacheSizeInbox    = 0x02,
    /** Volume cache size is 4MB */
    SSI_VolCacheSize4MB      = 0x04,
    /** Volume cache size is 16MB */
    SSI_VolCacheSize16MB     = 0x10
} SSI_VolCacheSize;

/**
 * @enum    SSI_RaidLevel
 *
 * @brief   Values that represent various Raid Levels.
**/
typedef enum _SSI_RaidLevel
{
    /** Invalid RaidLevel */
    SSI_RaidInvalid = 0x00,
    /** RAID 0 - striping*/
    SSI_Raid0  = 0x01,
    /** RAID 1 - mirroring*/
    SSI_Raid1  = 0x02,
    /** RAID 10 - striping + mirroring*/
    SSI_Raid10 = 0x04,
    /** RAID 5 - striping with parity*/
    SSI_Raid5  = 0x08,
    /** RAID 6 - striping, parity, and data-recovery*/
    SSI_Raid6  = 0x10,
    /** Unknown Raid level */
    SSI_RaidUnknown = 0x80000000,
} SSI_RaidLevel;

/**
 * @enum    SSI_StripSize
 *
 * @brief   Values that represent SSI_StripSize.
**/
typedef enum _SSI_StripSize
{
    /** Unknown stripe size */
    SSI_StripSizeUnknown    = 0x0000,
    /** 2KB stripe size */
    SSI_StripSize2kB        = 0x0001,
    /** 4KB stripe size */
    SSI_StripSize4kB        = 0x0002,
    /** 8KB stripe size */
    SSI_StripSize8kB        = 0x0004,
    /** 16KB stripe size */
    SSI_StripSize16kB       = 0x0008,
    /** 32KB stripe size */
    SSI_StripSize32kB       = 0x0010,
    /** 64KB stripe size */
    SSI_StripSize64kB       = 0x0020,
    /** 128KB stripe size */
    SSI_StripSize128kB      = 0x0040,
    /** 256KB stripe size */
    SSI_StripSize256kB      = 0x0080,
    /** 512KB stripe size */
    SSI_StripSize512kB      = 0x0100,
    /** 1MB stripe size */
    SSI_StripSize1MB        = 0x0200,
    /** 2MB stripe size */
    SSI_StripSize2MB        = 0x0400,
    /** 4MB stripe size */
    SSI_StripSize4MB        = 0x0800,
    /** 8MB stripe size */
    SSI_StripSize8MB        = 0x1000,
    /** 16MB stripe size */
    SSI_StripSize16MB       = 0x2000,
    /** 32MB stripe size */
    SSI_StripSize32MB       = 0x4000,
    /** 64MB stripe size */
    SSI_StripSize64MB       = 0x8000,
} SSI_StripSize;

/**
 * @enum    SSI_Rwh
 *
 * @brief    Close Raid Write Hole Policy.
**/
typedef enum _SSI_RwhPolicy
{
    SSI_RwhOff = 0,
    SSI_RwhDistributed = 1,
    SSI_RwhJournalingDrive = 2,
    SSI_RwhInvalid = 3,

} SSI_RwhPolicy;

/**
 * @enum    SSI_DeviceType
 *
 * @brief   Values that represent Device types.
**/
typedef enum _SSI_DeviceType
{
    /** Device type is unknown */
    SSI_DeviceTypeUnknown       = 0,
    /** Controller type */
    SSI_DeviceTypeController    = 1,
    /** End device type */
    SSI_DeviceTypeEndDevice     = 2,
    /** Routing device type */
    SSI_DeviceTypeRoutingDevice = 3,
} SSI_DeviceType;

/**
 * @enum    SSI_ScopeType
 *
 * @brief   Values that represent various scopes in SSI.
**/
typedef enum _SSI_ScopeType
{
    /** Scope is none */
    SSI_ScopeTypeNone               = 0,
    /** Controller Scope for Direct attached */
    SSI_ScopeTypeControllerDirect   = 1,
    /** Controller Scope for Controller complete */
    SSI_ScopeTypeControllerAll      = 2,
    /** RaidInfo Scope */
    SSI_ScopeTypeRaidInfo           = 3,
    /** EndDevice Scope */
    SSI_ScopeTypeEndDevice          = 4,
    /** Routing Device Scope*/
    SSI_ScopeTypeRoutingDevice      = 5,
    /** Port Scope */
    SSI_ScopeTypePort               = 6,
    /** Phy Scope */
    SSI_ScopeTypePhy                = 7,
    /** Array Scope */
    SSI_ScopeTypeArray              = 8,
    /** Enclosure Scope */
    SSI_ScopeTypeEnclosure          = 9,
} SSI_ScopeType;

/**
 * @struct  SSI_ScsiAddress
 *
 * @brief   Structure to represent the Scsi Address in SSI.
**/
typedef struct _SSI_ScsiAddress
{
    /** SCSI host */
    SSI_Uint8 host;
    /** SCSI bus */
    SSI_Uint8 bus;
    /** SCSI target */
    SSI_Uint8 target;
    /** SCSI lun */
    SSI_Uint8 lun;
} SSI_ScsiAddress;

/**
 * @struct  SSI_Address
 *
 * @brief   Structure to represent Address in SSI.
**/
typedef struct _SSI_Address
{
    /** If true, scsiAddress is valid */
    //SSI_Bool scsiAddressPresent;
    /** SCSI address */
    SSI_ScsiAddress scsiAddress;
    /** If true, sasAddress is valid */
    SSI_Bool sasAddressPresent;
    /** SAS address */
    SSI_Uint64 sasAddress;
} SSI_Address;

/**
 * @struct  SSI_SystemInfo
 *
 * @brief   Structure defines overall library, driver info.
**/
typedef struct _SSI_SystemInfo
{
    /** Major version of the library's interface */
    SSI_Uint32 interfaceVersionMajor;
    /** Minor version of the library's interface */
    SSI_Uint32 interfaceVersionMinor;
    /** Library version */
    SSI_Char libraryVersion[SSI_SOFTWARE_VERSION_LENGTH];
    /** Maximum Sessions */
    SSI_Uint32 maxSessions;
    /** If true, library supports #SSI_SetVolCacheSize function */
    SSI_Bool setVolCacheSizeSupport;
    /** If true, library supports #SSI_PassthroughCommand function */
    SSI_Bool passthroughCommandSupport;
} SSI_SystemInfo;

/**
 * @struct  SSI_RaidInfo
 *
 * @brief   Structure that defines Raid specific data.
**/
typedef struct _SSI_RaidInfo
{
    /** Handle to the Raid Information structure represented by this table */
    SSI_Handle raidHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Maximum disks allowed per array */
    SSI_Uint32 maxDisksPerArray;
    /** Maximum disks allowed to be used as array members */
    SSI_Uint32 maxRaidDisksSupported;
    /** Maximum volumes per array */
    SSI_Uint32 maxVolumesPerArray;
    /** Maximum volumes that may be created */
    SSI_Uint32 maxVolumesPerHba;
    /** If true, controller supports RAID */
    SSI_Bool raidEnabled;
    /** If true, spare disks do not have to be associated with a specific array */
    SSI_Bool supportsGlobalSpare;
    /** If true, spare disks can be assigned on a specific volume */
    SSI_Bool supportsDedicatedSpare;
    /** If true, volumes can be created using data from an existing member disk */
    SSI_Bool supportsCreateFromExisting;
    /** If true, controller supports empty arrays (arrays with no volumes) */
    SSI_Bool supportsEmptyArrays;
} SSI_RaidInfo;

/**
 * @enum    SSI_ControllerType
 *
 * @brief   Values that represents the Controller types.
**/
typedef enum _SSI_ControllerType
{
    /** Unknown controller type */
    SSI_ControllerTypeUnknown = 0,
    /** AHCI (SATA) controller */
    SSI_ControllerTypeAHCI    = 1,
    /** SCU (SAS) controller */
    SSI_ControllerTypeSCU     = 2,
    /** NVMe controller */
    SSI_ControllerTypeNVME    = 3,
    /** VMD controller */
    SSI_ControllerTypeVMD     = 4
} SSI_ControllerType;

/**
 * @struct  SSI_HardwareVersion
 *
 * @brief   Strucuture to Define Controller hardware identification(PCI).
**/
typedef struct _SSI_HardwareVersion
{
    /** Vendor id of device, as reported by PCI enumeration */
    SSI_Uint16 vendorId;
    /** Device id of device, as reported by PCI enumeration */
    SSI_Uint16 deviceId;
    /** Subsystem id of device, as reported by PCI enumeration */
    SSI_Uint16 subSystemId;
    /** Hardware revision ID of device, as reported by PCI enumeration */
    SSI_Uint8  hardwareRevisionId;
    /** Subclass code of device, as reported by PCI enumeration */
    SSI_Uint8  subClassCode;
    /** Subvendor id of device, as reported by PCI enumeration */
    SSI_Uint16 subVendorId;
} SSI_HardwareVersion;

/**
 * @struct  SSI_ControllerInfo
 *
 * @brief   Structure to define the Controller Information.
**/
typedef struct _SSI_ControllerInfo
{
    /** handle to controller */
    SSI_Handle controllerHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** address of controller */
    SSI_Address controllerAddress;
    /** name of controller */
    SSI_Char controllerName[SSI_CONTROLLER_NAME_LENGTH];
    /** type of controller */
    SSI_ControllerType controllerType;
    /** handle to Raid Info for the controller */
    SSI_Handle raidInfoHandle;
    /** driver version string */
    SSI_Char driverVer[SSI_SOFTWARE_VERSION_LENGTH];
    /** OROM/EFI version string */
    SSI_Char prebootManagerVer[SSI_SOFTWARE_VERSION_LENGTH];
    /** Hardware-specific identification info */
    SSI_HardwareVersion hardwareVer;

    /** If true, OROM/EFI has been loaded */
    SSI_Bool prebootManagerLoaded;
    /** If true, OROM supports system volumes greater than 2 TB */
    SSI_Bool twoTbVolumePrebootSupport;
    /** If true, OROM supports system disks greater than 2TB */
    SSI_Bool twoTbDiskPrebootSupport;
    /** If true, arrays may not be created with a mix of internal/external disks */
    SSI_Bool disableESataSpanning;

    /** If true, controller is using H/W XOR acceleration */
    SSI_Bool xorSupported;
    /** If true, controller is using NVSRAM */
    SSI_Bool nvsramSupported;

    /** If true, #SSI_PhyLocate function supports phys attached to this controller */
    SSI_Bool phyLocateSupport;
    /** If true, #SSI_DiskUnlock function supports disks attached to this controller */
    SSI_Bool diskUnlockSupport;
    /** If true, #SSI_AssignStoragePool function supports devices attached to this controller */
    SSI_Bool assignStoragePoolSupport;
    /** If true, #SSI_MarkAsNormal function supports devices attached to this controller */
    SSI_Bool markAsNormalSupport;
    /** If true, #SSI_VolumeDelete may be applied to this controller's volumes */
    SSI_Bool volumeDeleteSupport;
    /** If true, #SSI_ArrayCreate may be applied to this controller's volumes */
    SSI_Bool arrayCreateSupport;
    /** If true, #SSI_VolumeModify may be applied to this controller's volumes */
    SSI_Bool volumeModifySupport;
    /** If true, #SSI_VolumeRename may be applied to this controller's volumes */
    SSI_Bool volumeRenameSupport;
    /** If true, #SSI_AddDisksToArray may be applied to this controller's volumes */
    SSI_Bool addDisksToArraySupport;
    /** If true, #SSI_VolumeSetCachePolicy may be applied to this controller's volumes */
    SSI_Bool volumeSetCachePolicySupport;
    /** If true, #SSI_VolumeCancelVerify may be applied to this controller's volumes */
    SSI_Bool volumeCancelVerifySupport;
    /** If true, #SSI_MarkAsSpare may be applied to devices attached to this controller */
    SSI_Bool markAsSpareSupport;
    /** If true, Enabling or disabling read patrol is supported for this controller. */
    SSI_Bool readPatrolSupport;
    /** If true, Read Patrol is enabled for this controller */
    SSI_Bool readPatrolEnabled;
    /** If true, H/W XOR is enabled for this controller */
    SSI_Bool xorEnabled;

} SSI_ControllerInfo;

/**
 * @enum    SSI_PhyProtocol
 *
 * @brief   Values that represent Phy Protocol.
**/
typedef enum _SSI_PhyProtocol
{
    /** Phy protocol could not be determined */
    SSI_PhyProtocolUnknown = 0,
    /** SATA protocol */
    SSI_PhyProtocolSATA    = 1,
    /** SAS management protocol */
    SSI_PhyProtocolSMP     = 2,
    /** Serial tunneling protocol */
    SSI_PhyProtocolSTP     = 3,
    /** Serial SCSI protocol */
    SSI_PhyProtocolSSP     = 4,
    /** NVMe protocol */
    SSI_PhyProtocolNVME = 5
} SSI_PhyProtocol;

/**
 * @enum    SSI_PhySpeed
 *
 * @brief   Values that represent Phy link Speed.
**/
typedef enum _SSI_PhySpeed
{
    /** Link speed is unknown */
    SSI_PhySpeedUnknown   = 0,
    /** Generation 1 speed (1.5 Gbps) */
    SSI_PhySpeed_1_5_GEN1 = 1,
    /** Generation 2 speed (3.0 Gbps) */
    SSI_PhySpeed_3_0_GEN2 = 2,
    /** Generation 3 speed (6.0 Gbps) */
    SSI_PhySpeed_6_0_GEN3 = 3
} SSI_PhySpeed;

/**
 * @struct  SSI_PhyInfo
 *
 * @brief   Structure that represents Phy Information in SSI.
**/
typedef struct _SSI_PhyInfo
{
    /** Handle to phy */
    SSI_Handle phyHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Phy's address */
    SSI_Address phyAddress;
    /** Indicates which phy on the parent device this represents */
    SSI_Uint32 phyNumber;
    /** Connection protocol */
    SSI_PhyProtocol protocol;
    /** Logical port object that identifies connection between phys.
        Value is SSI_INVALID_HANDLE if phy is not connected. */
    SSI_Handle associatedPort;

    /** Type of device that phy is contained in */
    SSI_DeviceType deviceType;
    /** Handle to the device that phy is contained in */
    SSI_Handle deviceHandle;

    /** If true, phy is external */
    SSI_Bool isExternal;
    /** If true, hot plug even can be detected */
    SSI_Bool hotPlugCap;

    /** Minimum hardware speed phy can operate at */
    SSI_PhySpeed minHWLinkSpeed;
    /** Maximum hardware speed phy can operate at */
    SSI_PhySpeed maxHWLinkSpeed;
    /** Minimum programmed speed phy can operate at */
    SSI_PhySpeed minLinkSpeed;
    /** Maximum programmed speed phy can operate at */
    SSI_PhySpeed maxLinkSpeed;
    /** Negotiated link speed */
    SSI_PhySpeed negotiatedLinkSpeed;

    /** If true, following counts contain valid */
    SSI_Bool   countsValid;
    /** Number of invalid DWORDs that have been received since PWR reset */
    SSI_Uint32 invalildDWORDsCount;
    /** Number of frames received */
    SSI_Uint32 receivedFramesCount;
    /** Number of frames transmitted */
    SSI_Uint32 transmittedFramesCount;
    /** Number of DWORDs received */
    SSI_Uint32 receivedDWORDsCount;
    /** Number of DWORDs transmitted */
    SSI_Uint32 transmittedDWORDsCount;
    /** Number of times the py has restarted the link reset sequence due to lost DWORD synchronization */
    SSI_Uint32 syncLostCount;
    /** Number of DWORDs containing running disparity errors */
    SSI_Uint32 runningDisparityErrorsCount;
    /** Number of CRC errors */
    SSI_Uint32 crcErrorsCount;
    /** Number of short frames received */
    SSI_Uint32 shortFramesReceivedCount;
    /** Number of done frames received */
    SSI_Uint32 doneFrameReceivedCount;
    /** Number of times the phy reset problem has occurred */
    SSI_Uint32 PHYResetCount;
} SSI_PhyInfo;

/**
 * @struct  SSI_PortInfo
 *
 * @brief   Structure for Port Information.
**/
typedef struct _SSI_PortInfo
{
    /** Port Handle */
    SSI_Handle portHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Port Address */
    SSI_Address portAddress;
    /** Width of the port defined by the number of phys - x1, x2 or x4 */
    SSI_Uint32 numPhys;
    /** Local device type */
    SSI_DeviceType localDeviceType;
    /** Handle to the local device */
    SSI_Handle localDeviceHandle;
    /** Port that this is connected to on the remote device. */
    SSI_Handle connectedToPort;
} SSI_PortInfo;

/**
 * @struct  SSI_EnclosureInfo
 *
 * @brief   Defines the Enclosure Information.
**/
typedef struct _SSI_EnclosureInfo
{
    /** Enclosure Handle */
    SSI_Handle enclosureHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Unique Key for enclosure lookups */
    SSI_Uint32 enclosureKey;
    /** Vendor identification string */
    SSI_Char vendorInfo[SSI_VENDOR_LENGTH];
    /** Product identification string */
    SSI_Char productId[SSI_PRODUCT_ID_LENGTH];
    /** Product revision string */
    SSI_Char productRev[SSI_PRODUCT_REV_LENGTH];
    /** Unique logical identifier of the enclosure */
    SSI_Char logicalId[SSI_LOGICAL_ID_LENGTH];
    /** Number of processors in the enclosure */
    SSI_Uint32 processorCount;
    /** Number of sub-enclosures in the enclosure */
    SSI_Uint32 subenclosureCount;
    /** Number of elements in the enclosure */
    SSI_Uint32 elementCount;
    /** If the enclosure supports slot information, number of slots in the enclosure */
    SSI_Uint32 numberOfSlots;
} SSI_EnclosureInfo;

/**
 * @enum    SSI_EndDeviceType
 *
 * @brief   Values that represent EndDevice(Disk) Type.
**/
typedef enum _SSI_EndDeviceType
{
    /** Device type cannot be determined or represented by other type in enumeration */
    SSI_EndDeviceTypeUnknown        = 0,
    /** Device that exposes itself as a disk (magnetic HDDs, SSDs) */
    SSI_EndDeviceTypeDisk           = 1,
    /** Non disk storage devices (CDROM, Blue Ray, DVD, and tape drives) */
    SSI_EndDeviceTypeNonDiskStorage = 2,
    /** Non storage-devices (enclosure elements, processor) */
    SSI_EndDeviceTypeNonStorage     = 3
} SSI_EndDeviceType;

/**
 * @enum    SSI_DiskType
 *
 * @brief   Values that represents the type of Disk.
**/
typedef enum _SSI_DiskType
{
    /** Disk cannot be determined or represented by other type in enumeration */
    SSI_DiskTypeUnknown = 0,
    /** SATA disk */
    SSI_DiskTypeSATA    = 1,
    /** SAS disk */
    SSI_DiskTypeSAS     = 2,
    /** NVMe disk */
    SSI_DiskTypeNVME    = 3,
    /** VMD disk  */
    SSI_DiskTypeVMD     = 4
} SSI_DiskType;

/**
 * @enum    SSI_DiskState
 *
 * @brief   Values that represents the state of the Disk.
**/
typedef enum _SSI_DiskState
{
    /** Disk is in an unknown (likely bad) state */
    SSI_DiskStateUnknown             = 0,
    /** Disk is offline */
    SSI_DiskStateOffline             = 1,
    /** Disk is not in system */
    SSI_DiskStateMissing             = 2,
    /** Disk is in failed (multiple read/write failure) state */
    SSI_DiskStateFailed              = 3,
    /** Disk is reporting a SMART event */
    SSI_DiskStateSmartEventTriggered = 4,
    /** Disk contains metadata not understood by driver */
    SSI_DiskStateConfigIsUprev       = 5,
    /** Normal, healthy state */
    SSI_DiskStateNormal              = 6,
    /** Security is enabled (unable to read disk) */
    SSI_DiskStateLocked              = 7,
} SSI_DiskState;

/**
 * @enum    SSI_DiskUsage
 *
 * @brief   Values that represents Disk Usage.
**/
typedef enum _SSI_DiskUsage
{
    /** Disk usage cannot be determined or represented by another usage enumeration */
    SSI_DiskUsageUnknown                  = 0,
    /** Disk is a member of an array */
    SSI_DiskUsageArrayMember              = 1,
    /** Pass through disk */
    SSI_DiskUsagePassThru                 = 2,
    /** Disk is a member of an offline array */
    SSI_DiskUsageOfflineArray             = 3,
    /** Disk is marked as a spare */
    SSI_DiskUsageSpare                    = 4,
    /** Disk is part of an array, but mounted as a pass through */
    SSI_DiskUsageArrayMemberReadOnlyMount = 5,
    /** Disk is mounted as a pass through but has meta data */
    SSI_DiskUsagePassThruReadOnlyMount    = 6,
    /** Disk is marked as a journaling */
    SSI_DiskUsageJournaling               = 7,
} SSI_DiskUsage;

/**
 * @struct  SSI_EndDeviceInfo
 *
 * @brief   Defines the EndDevice Information in SSI.
**/
typedef struct _SSI_EndDeviceInfo
{
    /** End device handle */
    SSI_Handle endDeviceHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Address of end device */
    SSI_Address endDeviceAddress;
    /** Type of end device */
    SSI_EndDeviceType deviceType;

    /** Handle to the raidinfo for this enddevice */
    SSI_Handle raidInfoHandle;

    /** Handle to the associated array */
    SSI_Handle arrayHandle;
    /** Handle to the associated controller */
    SSI_Handle controllerHandle;
    /** Handle to the associated enclosure */
    SSI_Handle enclosureHandle;
    /** Address of enclosure slot that the device is plugged into */
    SSI_Address slotAddress;
    /** Associated storage pool this device */
    SSI_Uint32 storagePool;

    /** Serial number reported by device */
    SSI_Char serialNo[SSI_END_DEVICE_SERIALNO_LENGTH];
    /** Model number of device */
    SSI_Char model[SSI_END_DEVICE_MODEL_LENGTH];
    /** Firmware revision of device */
    SSI_Char firmware[SSI_END_DEVICE_FIRMWARE_LENGTH];
    /** Disk type, if end device is a disk */
    SSI_DiskType diskType;
    /** State of device */
    SSI_DiskState state;
    /** (RAID) Usage of device */
    SSI_DiskUsage usage;
    /** Size of storage device in Bytes */
    SSI_Uint64 totalSize;
    /** Size of the storage device in blocks */
    SSI_Uint64 blockSize;
    /** Total blocks of storage device in bytes */
    SSI_Uint64 blocksTotal;
    /** Total blocks free in the storage device in bytes */
    SSI_Uint64 blocksFree;
    /** Write cache policy */
    SSI_WriteCachePolicy writeCachePolicy;
    /** If true, disk contains files that the system requires to boot */
    SSI_Bool systemDisk;
    /** If disk part of an enclosure and enclosure provides slot information, the slot number of the disk */
    SSI_Uint32 slotNumber;
    /** If true, disk is blinkable via SGPIO */
    SSI_Bool locateLEDSupport;
    /** If true, disk is visible to PreBoot OROM or EFI */
    SSI_Bool isPreBootVisible;
    /** Led state */
    SSI_Uint32 ledState;
    /** PCI Bus number */
    SSI_Uint64 systemIoBusNumber;
    /** PCI Slot number */
    SSI_Uint64 PCISlotNumber;
    /** Fultondale x8 disk */
    SSI_Bool Isx8A;
    /** Fultondale x8 disk */
    SSI_Bool Isx8B;
    /** VMD domain index, if enddevice is attached to VMD controller */
    SSI_Uint32 vmdDomain;
} SSI_EndDeviceInfo;

/**
 * @enum    SSI_RoutingDeviceType
 *
 * @brief   Values that represent type of routing device.
**/
typedef enum _SSI_RoutingDeviceType
{
    /** Routing device is unknown */
    SSI_RoutingDeviceTypeUnknown    = 0,
    /** Routing device is a port multiplier */
    SSI_RoutingDeviceTypeMultiplier = 1,
    /** Routing device is a SAS expander */
    SSI_RoutingDeviceTypeExpander   = 2,
} SSI_RoutingDeviceType;

/**
 * @enum    SSI_ExpanderType
 *
 * @brief   Values that represent type of Expander.
**/
typedef enum _SSI_ExpanderType
{
    /** Expander type cannot be determined */
    SSI_ExpanderTypeUnknown = 0,
    /** Edge expander device */
    SSI_ExpanderTypeEdge    = 1,
    /** fan out expander device */
    SSI_ExpanderTypeFanout  = 2,
} SSI_ExpanderType;

/**
 * @struct  SSI_RoutingDeviceInfo
 *
 * @brief   Defines the Routing device information for SSI.
**/
typedef struct _SSI_RoutingDeviceInfo
{
    /** Handle to routing device */
    SSI_Handle routingDeviceHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Address of routing device */
    SSI_Address routingDeviceAddress;
    /** Type of routing device */
    SSI_RoutingDeviceType routingDeviceType;
    /** Handle to the Enclosure */
    SSI_Handle enclosureHandle;
    /** The number of phys in the device */
    SSI_Uint32 numberPhys;
    /** Identifies vendor of subsystem (board or enclosure) */
    SSI_Char vendorId[SSI_VENDOR_LENGTH];
    /** Identifies type of subsytem (board or enclosure) */
    SSI_Char productId[SSI_PRODUCT_ID_LENGTH];
    /** Revision of subsystem (board or enclosure) */
    SSI_Char productRev[SSI_PRODUCT_REV_LENGTH];
    /** Vendor Id of routing device */
    SSI_Char componentVendorId[SSI_VENDOR_LENGTH];
    /** Device model number id of routing device */
    SSI_Char componentId[SSI_ROUTING_DEVICE_COMPONENT_LENGTH];
    /** Revision level of the routing device */
    SSI_Char componentRev[SSI_ROUTING_DEVICE_COMPONENT_REV_LENGTH];
    /** Expander type */
    SSI_ExpanderType expanderType;
    /** Broadcast changes originated by an expander device */
    SSI_Uint32 expanderChangeCount;
    /** Maximum number of expander route indices per phy for the expander device */
    SSI_Uint32 expanderRouteIndexes;
    /** If true, the management device server is a self-configuring expander device */
    SSI_Bool selfConfiguring;
} SSI_RoutingDeviceInfo;

/**
 * @enum    SSI_ArrayState
 *
 * @brief   Values that represent State of the Array.
**/
typedef enum _SSI_ArrayState
{
    /** State is unknown */
    SSI_ArrayStateUnknown = 0,
    /** Array is in a normal, stable state */
    SSI_ArrayStateNormal  = 1,
    /** Array is in the middle of an operation (e.g. migration, rebuild, initialization, verify)*/
    SSI_ArrayStateBusy    = 2
} SSI_ArrayState;

/**
 * @struct  SSI_ArrayInfo
 *
 * @brief   Defines the structure that represents the Array Information.
**/
typedef struct _SSI_ArrayInfo
{
    /** Handle to Array */
    SSI_Handle arrayHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Array name */
    SSI_Char name[SSI_ARRAY_NAME_LENGTH];
    /** State of array */
    SSI_ArrayState state;
    /** Size of array as sum of usable space on all array's drives in Bytes */
    SSI_Uint64 totalSize;
    /** Amount of free usable space on all array drives in Bytes */
    SSI_Uint64 freeSize;
    /** Write cache policy */
    SSI_WriteCachePolicy writeCachePolicy;
    /** Number of disks in array */
    SSI_Uint32 numDisks;
    /** Number of volumes in array */
    SSI_Uint32 numVolumes;
} SSI_ArrayInfo;

/**
 * @enum    SSI_VolumeState
 *
 * @brief   Values that represent the Volume State.
**/
typedef enum _SSI_VolumeState
{
/** Volume is in an unknown (bad) state */
    SSI_VolumeStateUnknown          = 0,
    /** Volume is normal, healthy, and fully operational */
    SSI_VolumeStateNormal           = 1,
    /** Volume is in an operational, reduced-redundancy state */
    SSI_VolumeStateDegraded         = 2,
    /** Volume is non-operational */
    SSI_VolumeStateFailed           = 3,
    /** System is initializing volumes redundancy */
    SSI_VolumeStateInitializing     = 4,
    /** Volume was degraded and system is now in the process of rebuilding */
    SSI_VolumeStateRebuilding       = 5,
    /** System is performing a verify operation on volume */
    SSI_VolumeStateVerifying        = 6,
    /** System is performing a verify and fix operation on volume */
    SSI_VolumeStateVerifyingAndFix  = 7,
    /** System is changing volume's structure */
    SSI_VolumeStateGeneralMigration = 8,
    /** One or more member disks are locked and volume is offline */
    SSI_VolumeStateLocked           = 9,
    /** One or more memberDisk of a NonRedundant Volume is in a Failed state */
    SSI_VolumeStateNonRedundantVolumeFailedDisk = 16,
} SSI_VolumeState;

/**
 * @struct  SSI_VolumeInfo
 *
 * @brief   Defines the Volume Information structure.
**/
typedef struct _SSI_VolumeInfo
{
    /** Volume handle */
    SSI_Handle volumeHandle;
    /** Unique Id for object */
    SSI_Uint32 uniqueId;
    /** Array handle */
    SSI_Handle arrayHandle;
    /** Ordinal number (index) of volume in array */
    SSI_Uint32 arrayOrdinal;
    /** Volume name */
    SSI_Char volumeName[SSI_VOLUME_NAME_LENGTH];
    /** Volume state */
    SSI_VolumeState state;
    /** Volume's RAID level */
    SSI_RaidLevel raidLevel;
    /** Volume's total size as exported to the O/S in Bytes */
    SSI_Uint64 totalSize;
    /** Volume's stripe size */
    SSI_StripSize stripSize;
    /** Number of disks used by volume */
    SSI_Uint32 numDisks;
    /** Migration progress (% = migrProgress / 0xFFFFFFFF) */
    SSI_Uint32 migrProgress;
    /** Volume cache policy */
    SSI_VolumeCachePolicy cachePolicy;
    /** Volume Close Raid Write Hole Policy */
    SSI_RwhPolicy rwhPolicy;
    /** If true, volume contains system data required for boot */
    SSI_Bool systemVolume;
    /** If true, volume is currently migrating */
    SSI_Bool migrating;
    /** If true, parity / mirroring is complete */
    SSI_Bool initialized;
    /** Number of parity errors found since last verify was started */
    SSI_Uint32 verifyErrors;
    /** Number of bad blocks found since last verify was started */
    SSI_Uint32 verifyBadBlocks;
    /** Volume's logical sector size */
    SSI_Uint32 logicalSectorSize;
    /** Volume's physical sector size */
    SSI_Uint32 physicalSectorSize;
    /** Journaling drive handle*/
    SSI_Handle journalingDriveHandle;
} SSI_VolumeInfo;

/**
 * @struct  SSI_RaidLevelInfo
 *
 * @brief   Defines the Raidlevel Information for a particular
 *          Raid level for a given controller.
**/
typedef struct _SSI_RaidLevelInfo
{
    /** If true, this RAID level is supported (by controller)*/
    SSI_Bool supported;
    /** The minimum number of disks required for this level */
    SSI_Uint32 minDisks;
    /** The maximum number of disks supported by this level */
    SSI_Uint32 maxDisks;
    /** The default stripe size used for this RAID level */
    SSI_StripSize defaultStripSize;
    /** Or'd RAID-levels indicating RAID levels user can migrate to */
    SSI_RaidLevel migrSupport;
    /** Or'd RAID-levels indicating which RAID levels require additional disks in order to migrate to */
    SSI_RaidLevel migrDiskAdd;
    /** If true, indicates this level requires an even number of disks */
    SSI_Bool evenDiskCount;
    /** If true, indicates this level requires an odd number of disks */
    SSI_Bool oddDiskCount;
    /** Or'd stripe sizes supported by this RAID level */
    SSI_StripSize stripSizesSupported;
} SSI_RaidLevelInfo;

/**
 * @struct  SSI_DiskUnlockInfo
 *
 * @brief   Defines the DiskUnlock Information for SSI.
**/
typedef struct _SSI_DiskUnlockInfo
{
    /** Disk Password */
    SSI_Char diskPassword[SSI_DISK_PASSWORD_LENGTH];
    /** If true, indicates the password is user level */
    SSI_Bool userLevelPassword;       //true = user, false = master
} SSI_DiskUnlockInfo;

/**
 * @struct  SSI_CreateFromDisksParams
 *
 * @brief   Defines the Create From Disks Parameters.
**/
typedef struct _SSI_CreateFromDisksParams
{
    /** Array of all disks used to create volume */
    SSI_Handle *disks;
    /** Number of disks in disks field */
    SSI_Uint32     numDisks;
    /** If data shall be migrated from disk, handle of disk */
    SSI_Handle  sourceDisk;
    /** New volume name */
    SSI_Char       volumeName[SSI_VOLUME_NAME_LENGTH];
    /** Stripe size of new volume */
    SSI_StripSize  stripSize;
    /** Raid level of new volume */
    SSI_RaidLevel  raidLevel;
    /** Size in bytes of new volume */
    SSI_Uint64     sizeInBytes;
    /** Close Write Hole Policy */
    SSI_RwhPolicy rwhPolicy;
    /**Journaling drive handle for Raid Write Hole policy */
    SSI_Handle journalingDrive;
} SSI_CreateFromDisksParams;

/**
 * @struct  SSI_CreateFromArrayParams
 *
 * @brief   Defines Create from Array parameters.
**/
typedef struct _SSI_CreateFromArrayParams
{
    /** Array handle */
    SSI_Handle    arrayHandle;
    /** New volume name */
    SSI_Char      volumeName[SSI_VOLUME_NAME_LENGTH];
    /** Stripe size to use for volume */
    SSI_StripSize stripSize;
    /** RAID level for new volume */
    SSI_RaidLevel raidLevel;
    /** Size in bytes of new volume. 0 - use all available space */
    SSI_Uint64    sizeInBytes;
    /** Close Write Hole Policy */
    SSI_RwhPolicy rwhPolicy;
    /**Journaling drive handle for Raid Write Hole policy*/
    SSI_Handle journalingDrive;
} SSI_CreateFromArrayParams;

/**
 * @struct  SSI_ModifyParams
 *
 * @brief   Defines Volume Modify parameters.
**/
typedef struct _SSI_RaidLevelModifyParams
{
    /** Proposed stripe size */
    SSI_StripSize newStripSize;
    /** Proposed RAID level */
    SSI_RaidLevel newRaidLevel;
    /** Proposed size */
    SSI_Uint64    newSizeInBytes;
    /** Handle for Disks to be added to volume */
    SSI_Handle *diskHandles;
    /** Count of disks to be added to volume */
    SSI_Uint32 diskHandleCount;
} SSI_RaidLevelModifyParams;

/**
 * @enum    SSI_DataDirection
 *
 * @brief   Values that represent Data direction.
**/
typedef enum _SSI_DataDirection
{
    /** Pass through contains no data */
    SSI_DataDirectionNoData = 0,
    /** Read pass through command */
    SSI_DataDirectionRead = 1,
    /** Write pass through command */
    SSI_DataDirectionWrite = 2,
} SSI_DataDirection;

/**
 * @enum    SSI_StorageArea
 *
 * @brief   Values that represent Storage area.
**/
typedef enum _SSI_StorageArea
{
    /** Invalid area */
    SSI_StorageAreaInvalid = 0,
    /** CIM area */
    SSI_StorageAreaCim  = 1,
} SSI_StorageArea;

/**
 * @fn  SSI_API SSI_Status SsiInitialize()
 *
 * @brief   Initializes the SSI library and must be called prior to any other API call.
 *
 * @return  #SSI_StatusOk, #SSI_StatusDuplicate, #SSI_StatusInsufficientPrivileges, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiInitialize();

/**
 * @fn  SSI_API SSI_Status SsiFinalize()
 *
 * @brief   Frees library resources.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInternalError.
**/
SSI_API SSI_Status SsiFinalize();

/**
 * @fn  SSI_API SSI_Status SsiSessionOpen(SSI_Handle *session)
 *
 * @brief   Captures a snapshot of the system.
 *
 * @param [in,out]  session If non-null, the session.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInsufficientResources, #SSI_StatusInvalidParameter.
**/
SSI_API SSI_Status SsiSessionOpen(SSI_Handle *session);

/**
 * @fn  SSI_API SSI_Status SsiSessionClose(SSI_Handle session)
 *
 * @brief   Frees snapshot associated with session.
 *
 * @param   session The session handle.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidParameter.
**/
SSI_API SSI_Status SsiSessionClose(SSI_Handle session);

/**
 * @fn  SSI_API SSI_Status SsiGetEventHandle(SSI_Handle *eventHandle)
 *
 * @brief   Ssi get event handle.
 *
 * @param [in,out]  eventHandle If non-null, handle of the event.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetEventHandle(SSI_Handle *eventHandle);

/**
 * @fn  SSI_API SSI_Status SsiFreeEventHandle(SSI_Handle eventHandle)
 *
 * @brief   Ssi free event handle.
 *
 * @param   eventHandle Handle of the event.
 *
 * @return  .
**/
SSI_API SSI_Status SsiFreeEventHandle(SSI_Handle eventHandle);

/**
 * @fn  SSI_API SSI_Status SsiEventWait(SSI_Uint32 timeout, SSI_Handle eventHandle)
 *
 * @brief   Ssi event wait.
 *
 * @param   timeout     The timeout.
 * @param   eventHandle Handle of the event.
 *
 * @return  .
**/
SSI_API SSI_Status SsiEventWait(SSI_Uint32 timeout, SSI_Handle eventHandle);

/**
 * @fn  SSI_API SSI_Status SsiGetSystemInfo(SSI_SystemInfo *systemInfo)
 *
 * @brief   Ssi get system information.
 *
 * @param [in,out]  systemInfo  If non-null, information describing the system.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetSystemInfo(SSI_SystemInfo *systemInfo);

/**
 * @fn  SSI_API SSI_Status SsiGetRaidInfoHandles(SSI_Handle session, SSI_Handle *handleList,
 *      SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get raid information handles.
 *
 * @param   session             The session.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetRaidInfoHandles(SSI_Handle session, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetRaidInfo(SSI_Handle session, SSI_Handle handle,
 *      SSI_RaidInfo *raidInfo)
 *
 * @brief   Ssi get raid information.
 *
 * @param   session             The session.
 * @param   handle              The handle.
 * @param [in,out]  raidInfo    If non-null, information describing the raid.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetRaidInfo(SSI_Handle session, SSI_Handle handle, SSI_RaidInfo *raidInfo);

/**
 * @fn  SSI_API SSI_Status SsiGetControllerHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get controller handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetControllerHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetControllerInfo(SSI_Handle session, SSI_Handle handle,
 *      SSI_ControllerInfo *controllerInfo)
 *
 * @brief   Ssi get controller information.
 *
 * @param   session                 The session.
 * @param   handle                  The handle.
 * @param [in,out]  controllerInfo  If non-null, information describing the controller.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetControllerInfo(SSI_Handle session, SSI_Handle handle, SSI_ControllerInfo *controllerInfo);

/**
 * @fn  SSI_API SSI_Status SsiGetPhyHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get phy handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetPhyHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetPhyInfo(SSI_Handle session, SSI_Handle phyHandle,
 *      SSI_PhyInfo *info)
 *
 * @brief   Ssi get phy information.
 *
 * @param   session         The session.
 * @param   phyHandle       Handle of the phy.
 * @param [in,out]  info    If non-null, the information.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetPhyInfo(SSI_Handle session, SSI_Handle phyHandle, SSI_PhyInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetPortHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get port handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetPortHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetPortInfo(SSI_Handle session, SSI_Handle portHandle,
 *      SSI_PortInfo *info)
 *
 * @brief   Ssi get port information.
 *
 * @param   session         The session.
 * @param   portHandle      Handle of the port.
 * @param [in,out]  info    If non-null, the information.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetPortInfo(SSI_Handle session, SSI_Handle portHandle, SSI_PortInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetEnclosureHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get enclosure handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetEnclosureHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetEnclosureInfo(SSI_Handle session, SSI_Handle enclosureHandle,
 *      SSI_EnclosureInfo *info)
 *
 * @brief   Ssi get enclosure information.
 *
 * @param   session         The session.
 * @param   enclosureHandle Handle of the enclosure.
 * @param [in,out]  info    If non-null, the information.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetEnclosureInfo(SSI_Handle session, SSI_Handle enclosureHandle, SSI_EnclosureInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetEndDeviceHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get end device handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetEndDeviceHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetEndDeviceInfo(SSI_Handle session, SSI_Handle endDeviceHandle,
 *      SSI_EndDeviceInfo *info)
 *
 * @brief   Ssi get end device information.
 *
 * @param   session         The session.
 * @param   endDeviceHandle Handle of the end device.
 * @param [in,out]  info    If non-null, the information.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetEndDeviceInfo(SSI_Handle session, SSI_Handle endDeviceHandle, SSI_EndDeviceInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetRoutingDeviceHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get routing device handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetRoutingDeviceHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetRoutingDeviceInfo(SSI_Handle session,
 *      SSI_Handle routingDeviceHandle, SSI_RoutingDeviceInfo *info)
 *
 * @brief   Ssi get routing device information.
 *
 * @param   session             The session.
 * @param   routingDeviceHandle Handle of the routing device.
 * @param [in,out]  info        If non-null, the information.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetRoutingDeviceInfo(SSI_Handle session, SSI_Handle routingDeviceHandle, SSI_RoutingDeviceInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetArrayHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get array handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetArrayHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle,
 *      SSI_ArrayInfo *info)
 *
 * @brief   Ssi get array information.
 *
 * @param   session         The session.
 * @param   arrayHandle     Handle of the array.
 * @param [in,out]  info    If non-null, the information.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetArrayInfo(SSI_Handle session, SSI_Handle arrayHandle, SSI_ArrayInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetVolumeHandles(SSI_Handle session, SSI_ScopeType scopeType,
 *      SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount)
 *
 * @brief   Ssi get volume handles.
 *
 * @param   session             The session.
 * @param   scopeType           Type of the scope.
 * @param   scopeHandle         Handle of the scope.
 * @param [in,out]  handleList  If non-null, list of handles.
 * @param [in,out]  handleCount If non-null, number of handles.
 *
 * @return  .
**/
SSI_API SSI_Status SsiGetVolumeHandles(SSI_Handle session, SSI_ScopeType  scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);

/**
 * @fn  SSI_API SSI_Status SsiGetVolumeInfo(SSI_Handle session, SSI_Handle volumeHandle,
 *      SSI_VolumeInfo *info)
 *
 * @brief   Retrieves information for the volume indicated by volumeHandle.
 *
 * @param   session         The session.
 * @param   volumeHandle    Handle of the volume.
 * @param   info            Volume Information
 *
 * @return  #SSI_Status
**/
SSI_API SSI_Status SsiGetVolumeInfo(SSI_Handle session, SSI_Handle volumeHandle, SSI_VolumeInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiGetRaidLevelInfo(SSI_Handle session, SSI_Handle raidInfoHandle,
 *      SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *info)
 *
 * @brief   Returns the number of RAID levels for this controller.
 *
 * @param   session         The session handle.
 * @param   raidInfoHandle  Handle of the raid information.
 * @param   raidLevel       The raid level for which the info has to be collected.
 * @param   info            RaidLevelInfo strucure to be filled with data
 *
 * @return  #SSI_Status.
**/
SSI_API SSI_Status SsiGetRaidLevelInfo(SSI_Handle session, SSI_Handle raidInfoHandle, SSI_RaidLevel raidLevel, SSI_RaidLevelInfo *info);

/**
 * @fn  SSI_API SSI_Status SsiDiskClearMetadata(SSI_Handle diskHandle)
 *
 * @brief   Clears the metadata off of a disk.
 *
 * @param   diskHandle  Handle of the disk.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusFailed,
 * @return  #SSI_StatusInvalidState.
**/
SSI_API SSI_Status SsiDiskClearMetadata(SSI_Handle diskHandle);

/**
 * @fn  SSI_API SSI_Status SsiDiskMarkAsSpare(SSI_Handle diskHandle, SSI_Handle arrayHandle)
 *
 * @brief   Sets a disk to be a spare.
 *
 * @param   diskHandle  Handle of the disk.
 * @param   arrayHandle Handle of the array.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidState, #SSI_StatusInvalidHandle,
 * @return  #SSI_StatusFailed, #SSI_StatusNotSupported .
**/
SSI_API SSI_Status SsiDiskMarkAsSpare(SSI_Handle diskHandle, SSI_Handle arrayHandle);

/**
 * @fn  SSI_API SSI_Status SsiDiskUnmarkAsSpare(SSI_Handle diskHandle)
 *
 * @brief   Marks a spare-disk as a pass-through.
 *
 * @param   diskHandle  Handle of the disk.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidState, #SSI_StatusInvalidHandle,
 * @return  #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiDiskUnmarkAsSpare(SSI_Handle diskHandle);

/**
 * @fn  SSI_API SSI_Status SsiDiskMarkAsNormal(SSI_Handle diskHandle)
 *
 * @brief   Marks a failed disk as healthy
 *
 * @param   diskHandle  Handle of the disk.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidState, #SSI_StatusInvalidHandle, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiDiskMarkAsNormal(SSI_Handle diskHandle);

/**
 * @fn  SSI_API SSI_Status SsiDiskUnlock(SSI_Handle diskHandle, SSI_DiskUnlockInfo *unlockInfo)
 *
 * @brief   Unlocks a locked disk
 *
 * @param   diskHandle          Handle of the disk.
 * @param   unlockInfo          Information describing the unlock.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidState, #SSI_Status_InvalidHandle,
 * @return  #SSI_StatusFailed, #SSI_StatusInvalidParameter, #SSI_StatusRetriesExceeded,
 * @return  #SSI_StatusWrongPassword #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiDiskUnlock(SSI_Handle diskHandle, SSI_DiskUnlockInfo *unlockInfo);

/**
 * @fn  SSI_API SSI_Status SsiDiskAssignStoragePool(SSI_Handle diskHandle, SSI_Uint8 storagePool)
 *
 * @brief   Assigns a disk to the specified storage pool.
 *
 * @param   diskHandle  Handle of the disk.
 * @param   storagePool The storage pool identifier
 .
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiDiskAssignStoragePool(SSI_Handle diskHandle, SSI_Uint8 storagePool);

/**
 * @fn  SSI_API SSI_Status SsiVolumeMarkAsNormal(SSI_Handle volumeHandle)
 *
 * @brief   Marks a failed volume as healthy
 *
 * @param   volumeHandle    Handle of the unhealthy volume.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return  #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeMarkAsNormal(SSI_Handle volumeHandle);

/**
 * @fn  SSI_API SSI_Status SsiVolumeRebuild(SSI_Handle volumeHandle, SSI_Handle diskHandle)
 *
 * @brief   Rebuilds a degraded volume using the specified disk.
 *
 * @param   volumeHandle    Handle of the degraded volume.
 * @param   diskHandle      Handle of the disk to use for rebuild.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return  #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiVolumeRebuild(SSI_Handle volumeHandle, SSI_Handle diskHandle);

/**
 * @fn  SSI_API SSI_Status SsiVolumeDelete(SSI_Handle volumeHandle)
 *
 * @brief   Deletes a volume.
 *
 * @param   volumeHandle    Handle of the volume.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return  #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeDelete(SSI_Handle volumeHandle);

/**
 * @fn  SSI_API SSI_Status SsiVolumeCreateFromDisks(SSI_CreateFromDisksParams params, SSI_Handle* volumeHandle)
 *
 * @brief   Creates a new volume.
 *
 * @param   params  Parameters for Create from disks.
 * @param   volumeHandle [out] The resulting volume handle.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidStripeSize, #SSI_StatusInvalidString,
 * @return  #SSI_StatusInvalidSize, #SSI_StatusInvalidRaidLevel, #SSI_StatusDataExceedsLimits, #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeCreateFromDisks(SSI_CreateFromDisksParams params, SSI_Handle* volumeHandle);

/**
 * @fn  SSI_API SSI_Status SsiVolumeCreate(SSI_CreateFromArrayParams params)
 *
 * @brief   Creates a volume on an existing array.
 *
 * @param   params  Parameters for Creating from Array.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidStripeSize, #SSI_StatusInvalidString,
 * @return  #SSI_StatusInvalidSize, #SSI_StatusInvalidRaidLevel, #SSI_StatusDataExceedsLimits, #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeCreate(SSI_CreateFromArrayParams params);

/**
 * @fn  SSI_API SSI_Status SsiRaidLevelModify(SSI_Handle volumeHandle, SSI_ModifyParams params)
 *
 * @brief   Modifies one or more components of a RAID volume.
 *
 * @param   volumeHandle    Handle of the volume.
 * @param   params          Modify parameters.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidStripeSize, #SSI_StatusInvalidSize,
 * @return  #SSI_StatusInvalidRaidLevel, #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiRaidLevelModify(SSI_Handle volumeHandle, SSI_RaidLevelModifyParams params);

/**
 * @fn  SSI_API SSI_Status SsiVolumeRename(SSI_Handle volumeHandle,
 *      const SSI_Char volumeName[SSI_VOLUME_NAME_LENGTH])
 *
 * @brief   Changes volume name.
 *
 * @param   volumeHandle    Handle of the volume.
 * @param   volumeName      Name of the volume.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidString, #SSI_StatusFailed,
 * @return  #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeRename(SSI_Handle volumeHandle, const SSI_Char volumeName[SSI_VOLUME_NAME_LENGTH]);

/**
 * @fn  SSI_API SSI_Status SsiAddDisksToArray(SSI_Handle arrayHandle, SSI_Handle *diskHandles,
 *      SSI_Uint32 diskHandleCount)
 *
 * @brief   Add one or more disks to an existing array.
 *
 * @param   arrayHandle         Handle of the array.
 * @param   diskHandles         The disk handles.
 * @param   diskHandleCount     Number of disk handles.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidParameter,
 * @return  #SSI_StatusBufferTooSmall, #SSI_StatusInvalidSize, #SSI_StatusDataExceedsLimits, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiAddDisksToArray(SSI_Handle arrayHandle, SSI_Handle *diskHandles, SSI_Uint32 diskHandleCount);

/**
 * @fn  SSI_API SSI_Status SsiExpandVolume(SSI_Handle volumeHandle, SSI_Uint64 newSizeMB)
 *
 * @brief   Expands a volume to consume free array space.
 *
 * @param   volumeHandle    Handle of the volume.
 * @param   newSizeMB       The new size in mb, 0 = all available space.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidSize,
 * @return  #SSI_StatusFailed, #SSI_StatusNotSupported .
**/
SSI_API SSI_Status SsiExpandVolume(SSI_Handle volumeHandle, SSI_Uint64 newSizeMB);

/**
 * @fn  SSI_API SSI_Status SsiVolumeSetCachePolicy(SSI_Handle volumeHandle,
 *      SSI_VolumeCachePolicy policy)
 *
 * @brief   Changes the cache policy for a given volume.
 *
 * @param   volumeHandle    Handle of the volume to modify.
 * @param   policy          The new cache policy.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return #SSI_InvalidParameter, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiVolumeSetCachePolicy(SSI_Handle volumeHandle, SSI_VolumeCachePolicy policy);

/**
 * @fn    SSI_API SSI_Status SsiVolumeSetRwhPolicy(SSI_Handle volumeHandle, SSI_Handle journalingDriveHandle, SSI_RwhPolicy policy)
 *
* @brief    Changes the raid write hole policy for a given volume.
 *
 * @param    volumeHandle    Handle of the volume to modify.
 * @param    journalingDriveHandle    Handle of the journaling drive to mark.
 * @param    policy            The new raid write hole policy.
 *
 * @return    #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return #SSI_InvalidParameter, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiVolumeSetRwhPolicy(SSI_Handle volumeHandle, SSI_Handle journalingDriveHandle, SSI_RwhPolicy policy);

/**
 * @fn  SSI_API SSI_Status SsiVolumeInitialize(SSI_Handle volumeHandle)
 *
 * @brief   Initializes redundant volume (adding parity / mirroring).
 *
 * @param   volumeHandle    Handle of the volume to initialize.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeInitialize(SSI_Handle volumeHandle);

/**
 * @fn  SSI_API SSI_Status SsiVolumeVerify(SSI_Handle volumeHandle, SSI_Bool repair)
 *
 * @brief   Looks for bad blocks and conflicts in redundant information.
 *
 * @param   volumeHandle    Handle of the volume to examine.
 * @param   repair          The repair if true, repair redundant information conflicts.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return  #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiVolumeVerify(SSI_Handle volumeHandle, SSI_Bool repair);

/**
 * @fn  SSI_API SSI_Status SsiVolumeCancelVerify(SSI_Handle handle)
 *
 * @brief   Cancel an inprocess verify operation.
 *
 * @param   handle  The handle to volume in verify or verify and repair state.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiVolumeCancelVerify(SSI_Handle handle);

/**
 * @fn  SSI_API SSI_Status SsiArraySetWriteCacheState(SSI_Handle arrayHandle,
 *      SSI_Bool cacheEnabled)
 *
 * @brief   Sets the write cache state for all disks associated with the given array.
 *
 * @param   arrayHandle     Handle of the array.
 * @param   cacheEnabled    If true, enables all array disks caches; else disables them.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusInvalidState,
 * @return #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiArraySetWriteCacheState(SSI_Handle arrayHandle, SSI_Bool cacheEnabled);

/**
 * @fn  SSI_API SSI_Status SsiPhyLocate(SSI_Handle phyHandle)
 *
 * @brief   Triggers hardware-specific illumination to locate a given phy.
 *
 * @param   phyHandle   Handle of the phy.
 * @param   mode        If SSI_True, illumination is asserted, else de-asserted.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusNotSupported, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiPhyLocate(SSI_Handle phyHandle, SSI_Bool mode);

/**
 * @fn  SSI_API SSI_Status SsiPortLocate(SSI_Handle portHandle)
 *
 * @brief   Triggers hardware-specific illumination to locate a given port.
 *
 * @param   portHandle   Handle of the port.
 * @param   mode        If SSI_True, illumination is asserted, else de-asserted.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidHandle, #SSI_StatusNotSupported, #SSI_StatusFailed.
**/
SSI_API SSI_Status SsiPortLocate(SSI_Handle portHandle, SSI_Bool mode);

/**
 * @fn  SSI_API SSI_Status SsiSetVolCacheSize(SSI_VolCacheSize cacheSize)
 *
 * @brief   Changes the ammount of memory used for volume cache
 *
 * @param   cacheSize   Size of the cache to use for Volume Cache.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidParameter, #SSI_StatusNotSupported,
 * @return  #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiSetVolCacheSize(SSI_VolCacheSize cacheSize);

/**
 * @fn  SSI_API SSI_Status SsiRescan()
 *
 * @brief   Force rescan for hardware changes.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusFailed, #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiRescan();

/**
 * @fn  SSI_API SSI_Status SsiPassthroughCommand(SSI_Handle deviceHandle,
 *      void* commandInformationUnit, void* dataBuffer, SSI_Uint32 dataBufferLength,
 *      SSI_DataDirection dataDirection)
 *
 * @brief   Sends a command directly to a device (SCSI SES request, SCSI disk request, etc)
 *
 * @param   deviceHandle                    Handle of the device.
 * @param [in,out]  commandInformationUnit  If non-null, the command information unit.
 * @param [in,out]  dataBuffer              If non-null, buffer for data data.
 * @param   dataBufferLength                Length of the data buffer.
 * @param   dataDirection                   The data direction.
 *
 * @return   #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusFailed, #SSI_StatusInvalidHandle, #SSI_StatusInvalidParameter,
 * @return   #SSI_StatusNotSupported.
**/
SSI_API SSI_Status SsiPassthroughCommand(SSI_Handle deviceHandle, void* commandInformationUnit, void* dataBuffer, SSI_Uint32 dataBufferLength, SSI_DataDirection dataDirection);

/**
 * @fn  SSI_API SSI_Status SsiReadStorageArea(SSI_Handle deviceHandle, SSI_StorageArea storageArea, void *buffer,
 *      SSI_Uint32 bufferLen)
 *
 * @brief   Read from specified storage area in metadata.
 *
 * @param   deviceHandle    Handle of the device
 * @param   storageArea     The storage area to read from.
 * @param [in,out]  buffer  If non-null, the buffer data.
 * @param   bufferLen       Length of the buffer in bytes.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidParameter, #SSI_StatusNotSupported,
 * @return #SSI_StatusFailed, #SSI_StatusInvalidState, #SSI_StatusDataExceedsLimits.
**/
SSI_API SSI_Status SsiReadStorageArea(SSI_Handle deviceHandle, SSI_StorageArea storageArea, void *buffer, SSI_Uint32 bufferLen);

/**
 * @fn  SSI_API SSI_Status SsiWriteStorageArea(SSI_Handle deviceHandle, SSI_StorageArea storageArea, void *buffer,
 *      SSI_Uint32 bufferLen)
 *
 * @brief   Writes contents of buffer to specified metadata storage area.
 *
 * @param   deviceHandle    Handle of the device
 * @param   storageArea     The storage area to write to.
 * @param [in,out]  buffer  If non-null, the buffer with data to write.
 * @param   bufferLen       Length of the buffer in number of bytes.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidParameter, #SSI_StatusNotSupported,
 * @return  #SSI_StatusFailed, #SSI_StatusInvalidState, #SSI_StatusDataExceedsLimits.
**/
SSI_API SSI_Status SsiWriteStorageArea(SSI_Handle deviceHandle, SSI_StorageArea storageArea, void *buffer, SSI_Uint32 bufferLen);

/**
 * @fn  SSI_API SSI_Status SsiReadPatrolSetState(SSI_Handle controllerHandle, SSI_Bool enable)
 *
 * @brief   Ssi read patrol set state.
 *
 * @param   controllerHandle    Handle of the controller.
 * @param   enable              Boolean to enable or disable the read ReadPatrol.
 *
 * @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidParameter, #SSI_StatusNotSupported,
 * @return  #SSI_StatusFailed, #SSI_StatusInvalidState
**/
SSI_API SSI_Status SsiReadPatrolSetState(SSI_Handle controllerHandle, SSI_Bool enable);

/**
* @fn  SSI_API SSI_Status SsiROHISetState(SSI_Handle controllerHandle, SSI_Bool enable)
*
* @brief   Ssi rebuild on hot insert set state.
*
* @param   controllerHandle    Handle of the controller.
* @param   enable              Boolean to enable or disable rebuild on hot insert.
*
* @return  #SSI_StatusOk, #SSI_StatusNotInitialized, #SSI_StatusInvalidParameter, #SSI_StatusNotSupported,
* @return  #SSI_StatusFailed, #SSI_StatusInvalidState
**/
SSI_API SSI_Status SsiROHISetState(SSI_Handle controllerHandle, SSI_Bool enable);

/**
 * @fn  SSI_API const SSI_Char * SsiGetLastErrorMessage()
 *
 * @brief   Ssi read last error message.
 *
 *
 * @return Error message string.

**/
SSI_API const SSI_Char * SsiGetLastErrorMessage();

#ifndef _MSC_VER
#pragma GCC visibility pop
#endif

#ifdef __cplusplus
}
#endif

#endif
