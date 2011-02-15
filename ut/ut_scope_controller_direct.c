// -*- mode: c; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
// ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "ssi.h"

const char *statusTable[] = {
    "SSI_StatusOk",
    "SSI_StatusInsufficientResources",
    "SSI_StatusInvalidParameter",
    "SSI_StatusInvalidHandle",
    "SSI_StatusInvalidStripSize",
    "SSI_StatusInvalidString",
    "SSI_StatusInvalidSize",
    "SSI_StatusInvalidRaidLevel",
    "SSI_StatusInvalidSession",
    "SSI_StatusTimeout",
    "SSI_StatusNotImplemented",
    "SSI_StatusDuplicate",
    "SSI_StatusNotInitialized",
    "SSI_StatusBufferTooSmall",
    "SSI_StatusNotSupported",
    "SSI_StatusFailed",
    "SSI_StatusInternalError",
    "SSI_StatusInvalidScope",
    "SSI_StatusInvalidState",
    "SSI_StatusRetriesExceeded",
    "SSI_StatusWrongPassword",
    "SSI_StatusDataExceedsLimits",
    "SSI_StatusInsufficientPrivileges"
};

#define min(__a, __b) \
    ({ typeof(__a) a = __a; \
       typeof(__b) b = __b; \
       a < b ? a : b; })

typedef SSI_Status (GetInfo)(SSI_Handle sessionHandle, SSI_Handle handle, void *info);
typedef SSI_Status (GetHandles)(SSI_Handle session, SSI_ScopeType scopeType, SSI_Handle scopeHandle, SSI_Handle *handleList, SSI_Uint32 *handleCount);
typedef void (ShowInfo)(void *info);

void printSessions(void);

const char *status2str(SSI_Status status)
{
    return statusTable[status];
}

void app_log(const char *log_text, ...)
{
    //return;
    va_list ap;
    fprintf(stderr, "APP");
    fprintf(stderr, " : ");
    va_start(ap, log_text);
    vfprintf(stderr, log_text, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    fflush(stderr);
}

#define HANDLE_LIST_SIZE    200

SSI_Status ReadWriteSsiStorage_v2_d2_d2(SSI_Handle sessionHandle)
{
    SSI_Status result;
    SSI_Handle handleList[HANDLE_LIST_SIZE];
    SSI_Uint32 handleCount=HANDLE_LIST_SIZE;

    char buffer[512];

    memset(buffer, 'a', sizeof(buffer));

/*  result = SsiGetVolumeHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    if (handleCount < 1)
        return SSI_StatusFailed;

    result = SsiReadStorageArea(handleList[0], SSI_StorageAreaCim, buffer, sizeof(buffer));
    app_log("Status = %d (%s)\n", result, status2str(result));

    result = SsiWriteStorageArea(handleList[0], SSI_StorageAreaCim, buffer, sizeof(buffer));
    app_log("Status = %d (%s)\n", result, status2str(result));
*/
    handleCount = 100;
    result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    if (handleCount < 1)
        return SSI_StatusFailed;

    result = SsiReadStorageArea(handleList[3], SSI_StorageAreaCim, buffer, sizeof(buffer));
    app_log("Status = %d (%s)\n", result, status2str(result));

    result = SsiWriteStorageArea(handleList[3], SSI_StorageAreaCim, buffer, sizeof(buffer));
    app_log("Status = %d (%s)\n", result, status2str(result));

    return result;
}

SSI_Status CreateVolume_a1_v1_d2(SSI_Handle sessionHandle)
{
    SSI_CreateFromDisksParams createFromDisksParams;
    SSI_Status result;
    SSI_Handle handleList[HANDLE_LIST_SIZE];
    SSI_Uint32 handleCount=HANDLE_LIST_SIZE;
    SSI_Handle diskHandleTable[HANDLE_LIST_SIZE];
    //SSI_Uint32 diskHandleCount=HANDLE_LIST_SIZE;

    (void)sessionHandle;
    SSI_EndDeviceInfo info;
    unsigned int i;

    result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    for (i = 3; i >= 2; i--)
    {
        SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList[i], &info);
        app_log("Info: %d, %s\n", i, info.serialNo);
    }
    createFromDisksParams.numDisks = 2;

    diskHandleTable[0] = handleList[3];
    diskHandleTable[1] = handleList[2];

    createFromDisksParams.disks = diskHandleTable;
    createFromDisksParams.sourceDisk = SSI_NULL_HANDLE;
    createFromDisksParams.stripSize = SSI_StripSize8kB;
    createFromDisksParams.raidLevel = SSI_Raid0;
    createFromDisksParams.sizeInBytes = 5000000000LL;
    strcpy(createFromDisksParams.volumeName, "Volume_0000");

    result = SsiVolumeCreateFromDisks(createFromDisksParams);
    app_log("Status = %d (%s)\n", result, status2str(result));

    return result;
}

SSI_Status CreateVolume_a1_v2_d2(SSI_Handle sessionHandle)
{
    SSI_CreateFromDisksParams createFromDisksParams;
    SSI_Status result;
    SSI_Handle handleList[HANDLE_LIST_SIZE];
    SSI_Uint32 handleCount=HANDLE_LIST_SIZE;
    SSI_Handle diskHandleTable[HANDLE_LIST_SIZE];
    //SSI_Uint32 diskHandleCount=HANDLE_LIST_SIZE;

    SSI_EndDeviceInfo info;
    unsigned int i;

    result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    for (i = 3; i >= 2; i--)
    {
        SsiGetEndDeviceInfo(sessionHandle, handleList[i], &info);
        app_log("Info: %d, %s\n", i, info.serialNo);
    }
    createFromDisksParams.numDisks = 2;

    diskHandleTable[0] = handleList[3];
    diskHandleTable[1] = handleList[2];

    createFromDisksParams.disks = diskHandleTable;
    createFromDisksParams.sourceDisk = SSI_NULL_HANDLE;
    createFromDisksParams.stripSize = SSI_StripSize8kB;
    createFromDisksParams.raidLevel = SSI_Raid0;
    createFromDisksParams.sizeInBytes = 5000000000LL;
    strcpy(createFromDisksParams.volumeName, "Volume_0000");

    result = SsiVolumeCreateFromDisks(createFromDisksParams);
    app_log("Status = %d (%s)\n", result, status2str(result));

    createFromDisksParams.disks = diskHandleTable;
    createFromDisksParams.sourceDisk = SSI_NULL_HANDLE;
    createFromDisksParams.stripSize = SSI_StripSize8kB;
    createFromDisksParams.raidLevel = SSI_Raid0;
    createFromDisksParams.sizeInBytes = 5000000000LL;
    strcpy(createFromDisksParams.volumeName, "Volume_0001");

    result = SsiVolumeCreateFromDisks(createFromDisksParams);
    app_log("Status = %d (%s)\n", result, status2str(result));

    return result;
}

SSI_Status DeleteArray_a1(SSI_Handle sessionHandle)
{
    SSI_Handle arr;
    SSI_Uint32 cnt = 1;
    SSI_Status result;

    result = SsiGetArrayHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, &arr, &cnt);
    app_log("Count = %d, Status = %d (%s)\n", cnt, result, status2str(result));

#if 0 /* APW */
    if (result == SSI_StatusOk)
    {
        result = SsiArrayDelete(arr);
        app_log("Status = %d (%s)\n", result, status2str(result));
    }
#endif /* APW */

    return result;
}

SSI_Status sessionTest(void)
{
    int i;
    SSI_Status result;
    SSI_Handle sessionHandle;

    SsiInitialize();
#define SESSION_LIMIT 6

    for (i = 0; i < SESSION_LIMIT + 2; i++)
    {
        result = SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));
        if (i < SESSION_LIMIT)
            assert(result == SSI_StatusOk);
        else
            assert(result == SSI_StatusInsufficientResources);
//      printSessions();
    }

    sessionHandle = 23;
    app_log("Try to free session handle = %u", sessionHandle);
    result = SsiSessionClose(sessionHandle);
    app_log("Status = %d (%s)\n", result, status2str(result));
    assert(result == SSI_StatusInvalidHandle);

    sessionHandle = 5;
        app_log("Try to free session handle = %u", sessionHandle);
        result = SsiSessionClose(sessionHandle);
    app_log("Status = %d (%s)\n", result, status2str(result));
    assert(result == SSI_StatusOk);

    sessionHandle = 1;
        app_log("Try to free session handle = %u", sessionHandle);
        result = SsiSessionClose(sessionHandle);
    app_log("Status = %d (%s)\n", result, status2str(result));
    assert(result == SSI_StatusOk);

        sessionHandle = 10;
        app_log("Try to free session handle = %u", sessionHandle);
        result = SsiSessionClose(sessionHandle);
    app_log("Status = %d (%s)\n", result, status2str(result));
    assert(result == SSI_StatusInvalidHandle);

    //printSessions();

        result = SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));
    assert(result == SSI_StatusOk);

        result = SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));
    assert(result == SSI_StatusOk);

        result = SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));
    assert(result == SSI_StatusInsufficientResources);

    result = SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));
    assert(result == SSI_StatusInsufficientResources);

    //printSessions();

    result = SsiFinalize();
    app_log("Status = %d (%s)\n", result, status2str(result));
    assert(result == SSI_StatusOk);

    //printSessions();

    result = SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));
    assert(result == SSI_StatusNotInitialized);

    return SSI_StatusOk;
}

SSI_Status devInfo(SSI_Handle sessionHandle, GetInfo *getInfo, GetHandles *getHandles, ShowInfo *showInfo, void *info)
{
    SSI_Status result;
    SSI_Handle handleList[HANDLE_LIST_SIZE];
    SSI_Uint32 handleCount=HANDLE_LIST_SIZE;

    unsigned int i;

    result = (*getHandles)(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    for (i = 0; i < handleCount; i++)
    {
        result = (*getInfo)(sessionHandle, handleList[i], (void *)info);
        assert(result == SSI_StatusOk);
        app_log("Info (%d):\n", i);
        showInfo(info);
    }

    return SSI_StatusOk;
}

/* macro to create test cases for different types of devices*/
#define MAKE_DEVTYPE_TC(devtype)                    \
    SSI_Status devtype##InfoTc(SSI_Handle sessionHandle)        \
    {                               \
        SSI_##devtype##Info info;               \
        memset(&info, 0, sizeof(info));             \
        devInfo(sessionHandle, (GetInfo *)SsiGet##devtype##Info, SsiGet##devtype##Handles, show##devtype##Info, (void *)&info); \
        return SSI_StatusOk;                    \
    }

/* generic functions to show short device info */
void showEndDeviceInfo(void *info)
{
    app_log("%s\n", ((SSI_EndDeviceInfo *)info)->serialNo);
}
void showPortInfo(void *info)
{
    app_log("%08x\n", ((SSI_PortInfo *)info)->portHandle);
}

void showPhyInfo(void *info)
{
    app_log("%d\n", ((SSI_PhyInfo *)info)->phyNumber);
}

void showControllerInfo(void *info)
{
    app_log("%s\n", ((SSI_ControllerInfo *)info)->controllerName);
}

void showArrayInfo(void *info)
{
    app_log("%08x\n", ((SSI_ArrayInfo *)info)->name);
}

void showVolumeInfo(void *info)
{
    app_log("%08x\n", ((SSI_VolumeInfo *)info)->volumeName);
}

void showEnclosureInfo(void *info)
{
    app_log("%08x\n", ((SSI_EnclosureInfo *)info)->productId);
}

void showRoutingDeviceInfo(void *info)
{
    app_log("%08x\n", ((SSI_RoutingDeviceInfo *)info)->productId);
}

MAKE_DEVTYPE_TC(EndDevice)
MAKE_DEVTYPE_TC(Port)
MAKE_DEVTYPE_TC(Phy)
MAKE_DEVTYPE_TC(Controller)
MAKE_DEVTYPE_TC(RoutingDevice)
MAKE_DEVTYPE_TC(Array)
MAKE_DEVTYPE_TC(Volume)
MAKE_DEVTYPE_TC(Enclosure)

#define SERIES_MAX 2

void allInfosTestTC()
{
    SSI_Status result = SSI_StatusNotSupported;
    SSI_Handle sessionHandle;

    int series;

#define USE_SESSION
    for (series = 0; series < SERIES_MAX; series++)
    {
        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));
        assert(result == SSI_StatusOk);

        sessionHandle = SSI_NULL_HANDLE;
#ifdef USE_SESSION
        if (series % 2)
        {
            result = SsiSessionOpen(&sessionHandle);
            app_log("Session handle = %u\n", sessionHandle);
            app_log("Status = %d (%s)\n", result, status2str(result));
            assert(result == SSI_StatusOk);
        }
#endif

        EndDeviceInfoTc(sessionHandle);

        PortInfoTc(sessionHandle);

        PhyInfoTc(sessionHandle);

        ControllerInfoTc(sessionHandle);

        RoutingDeviceInfoTc(sessionHandle);

        ArrayInfoTc(sessionHandle);

        VolumeInfoTc(sessionHandle);

        EnclosureInfoTc(sessionHandle);

#ifdef USE_SESSION
        if (series % 2)
        {
            result = SsiSessionClose(sessionHandle);
            app_log("Status = %d (%s)\n", result, status2str(result));
            assert(result == SSI_StatusOk);
        }
#endif

        result = SsiFinalize();
        app_log("Status = %d (%s)\n", result, status2str(result));
        assert(result == SSI_StatusOk);
    }
#ifdef USE_SESSION
#   undef USE_SESSION
#endif
}

SSI_Status ClearMetadataTC(SSI_Handle sessionHandle)
{
    SSI_Handle arrayList[HANDLE_LIST_SIZE];
    SSI_Uint32 arrayCount=HANDLE_LIST_SIZE;
    SSI_Handle diskList[HANDLE_LIST_SIZE];
    SSI_Uint32 diskCount=HANDLE_LIST_SIZE;
    SSI_Status result;
    SSI_EndDeviceInfo diskInfo;
    unsigned int i;

    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));
    assert(result == SSI_StatusOk);

    result = SsiGetArrayHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, arrayList, &arrayCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), arrayCount);
    assert(result == SSI_StatusOk);

    result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, diskList, &diskCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), diskCount);
    assert(result == SSI_StatusOk);

    app_log("Clear SPARES");
    for (i = 0; i < diskCount; i++) {
        /* get info of disk*/
        result = SsiGetEndDeviceInfo(sessionHandle, diskList[i], &diskInfo);
        app_log("Status = %d (%s)\n", result, status2str(result));
        assert(result == SSI_StatusOk);
        /* test only if spare */
        if (diskInfo.usage == SSI_DiskUsageSpare)
        {
            /* clear metadata */
            result = SsiDiskClearMetadata(diskList[i]);
            app_log("Status = %d (%s)\n", result, status2str(result));
            assert(result == SSI_StatusOk);
            /* verify action, get info and see if passthrough */
            result = SsiGetEndDeviceInfo(sessionHandle, diskList[i], &diskInfo);
            app_log("Status = %d (%s)\n", result, status2str(result));
            assert(result == SSI_StatusOk);
            assert(diskInfo.usage == SSI_DiskUsagePassThru);
        }
    }

    if (arrayCount == 0)
    {
        app_log("*************************************************************");
        app_log("WARNING!!!!!!! Test for ARRAYS skipped!");
        app_log("*************************************************************");
        return SSI_StatusOk;
    }

    app_log("Clear ARRAYS");
    for (i = 0; i < arrayCount; i++) {
        diskCount = HANDLE_LIST_SIZE;
        result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeArray, arrayList[i], diskList, &diskCount);
        app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), diskCount);
        assert(result == SSI_StatusOk);

        result = SsiDiskClearMetadata(diskList[0]);
        app_log("Status = %d (%s)\n", result, status2str(result));
        assert(result == SSI_StatusOk);
        result = SsiGetEndDeviceInfo(sessionHandle, diskList[0], &diskInfo);
        app_log("Status = %d (%s)\n", result, status2str(result));
        assert(result == SSI_StatusOk);
        assert(diskInfo.usage == SSI_DiskUsagePassThru);
    }

    return SSI_StatusOk;
}

SSI_Status verifyNameTC(void)
{
#if 0 /* APW */
    char *name1=(char *)"a\037b";
    char *name2=(char *)"a\176b"; //
    char *name3=(char *)"a\\b"; // backslash
    char *name4=(char *)"12345678901234567"; // 17 chars
    char *name5=(char *)"123456789 123456"; // 16 chars

    SSI_Bool result;
    result = validateVolumeName(name1);
    app_log("Result = %s\n", result == SSI_TRUE ? "true":"false");
    assert(result == SSI_FALSE);
    result = validateVolumeName(name2);
    app_log("Result = %s\n", result == SSI_TRUE ? "true":"false");
    assert(result == SSI_TRUE);
    result = validateVolumeName(name3);
    app_log("Result = %s\n", result == SSI_TRUE ? "true":"false");
    assert(result == SSI_FALSE);
    result = validateVolumeName(name4);
    app_log("Result = %s\n", result == SSI_TRUE ? "true":"false");
    assert(result == SSI_FALSE);
    result = validateVolumeName(name5);
    app_log("Result = %s\n", result == SSI_TRUE ? "true":"false");
    assert(result == SSI_TRUE);
#endif /* APW */

    return SSI_StatusOk;
}


int main()
{
    SSI_Uint32      i;
    SSI_Status      result = SSI_StatusNotSupported;

    SSI_Handle      sessionHandle;
#if 0 /* APW */
    SSI_Char        *name;
#endif /* APW */
    SSI_Handle      handleList[HANDLE_LIST_SIZE];
    SSI_Uint32      handleCount=HANDLE_LIST_SIZE;
    SSI_Uint32      handleListSize = HANDLE_LIST_SIZE;
    SSI_Handle      handle, handle1, handle2;
    SSI_Uint32      ctrlCount = HANDLE_LIST_SIZE;
    SSI_Handle      ctrlList[HANDLE_LIST_SIZE];

    SSI_VolumeInfo      volumeInfo;
    SSI_ArrayInfo       arrayInfo;
    SSI_ControllerInfo  controllerInfo;
    SSI_EndDeviceInfo   endDeviceInfo;
    SSI_PhyInfo     phyObject;
    SSI_PortInfo        portObject;
    SSI_EnclosureInfo   enclosureObject;
    SSI_RoutingDeviceInfo   routingDevObject;
#if 0 /* APW */
    SSI_Handle      diskHandleTable[3];
    SSI_Uint32      size;
#endif /* APW */
    SSI_Uint32      j;

/* passive_tests (does not change environment) */
//  allInfosTestTC();

    goto raidHandles;
    return ClearMetadataTC(SSI_NULL_HANDLE);

    goto allInfosTest;
    goto phyInfoTest;
    goto portInfoTest;
    goto endDevInfoTest;
    goto sessionTest2;
    goto sessionTest;
    goto msaStorageTest;
    goto mapFileTest;
    goto volumeRenameTest;
    goto volumeDeleteTest;
#if 0 /* APW */
    goto stopArrayTest;
#endif /* APW */
    goto arrayTest;
#if 0 /* APW */
    goto volumeAction;
#endif /* APW */
    goto volumeTest;
    goto volumeInfo;
    goto testBug;
    goto verifyName;
    goto raidHandles;
    goto endDeviceTestScopes;
    goto arrayActionSetWriteCache;
    goto volumeActionRebuild;
    goto eventTest;
    /* goto arrayAction; */
    goto phyTest;
    goto controllerTest;
    goto volumeTest2;
    goto enclosureTest;
    goto routingDeviceTest;
    goto phyTest;
    goto portInfoTest;
    goto enclosureTest;
    goto endDeviceTest;
    goto controllerTest;
    goto controllerTest;
    goto storagePoolTest;
    goto portTest;
    goto phyTest;
    goto volumePolicy;
#if 0 /* APW */
    goto volumeAction;
#endif /* APW */
    goto cmdTest;
#if 0 /* APW */
    goto utilsTest;
#endif /* APW */
    goto raidCapabilityTest;

allInfosTest:
    /* does not need initialization */
    allInfosTestTC();

    app_log("XXXXXXXX");

    goto end;

verifyName:
    /* does not need initialization */
    verifyNameTC();

    app_log("XXXXXXXXX");
    goto end;

msaStorageTest:
    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;
#ifdef USE_SESSION
    SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u",sessionHandle);
#endif
    result = ReadWriteSsiStorage_v2_d2_d2(sessionHandle);

    system("cat mdadm.conf.ssi");
    return 0;

//#define USE_SESSION
mapFileTest:;
    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;
#ifdef USE_SESSION
    SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u",sessionHandle);
#endif
    result = CreateVolume_a1_v2_d2(sessionHandle);

    system("cat mdadm.conf.ssi");

    app_log("Create test");
    assert(result == SSI_StatusOk);

    result = DeleteArray_a1(sessionHandle);

    system("cat mdadm.conf.ssi");

    app_log("Delete test");
    assert(result == SSI_StatusOk);

#ifdef USE_SESSION
    SsiSessionClose(sessionHandle);
#endif

    result = SsiFinalize();

    return result;

#if 0 /* APW */
stopArrayTest:;
    {
        handleCount = 10;

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        sessionHandle = SSI_NULL_HANDLE;
//        result = SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));

        result = SsiGetArrayHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

        if (handleCount)
        {
#if 1
            if (result == SSI_StatusOk )
            {
                handleCount = 10;

                result = SsiGetVolumeHandles(sessionHandle, SSI_ScopeTypeArray, handleList[0], handleList, &handleCount);
                app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));
            }
#endif
        stopMdArray(handleList[0]);
        }
    }
#endif /* APW */

volumeDeleteTest:;
    {
        handleCount = 10;

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        sessionHandle = SSI_NULL_HANDLE;
//        result = SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));

        result = SsiGetArrayHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

        if (handleCount)
        {
            if (result == SSI_StatusOk )
            {
                handleCount = 10;

                result = SsiGetVolumeHandles(sessionHandle, SSI_ScopeTypeArray, handleList[0], handleList, &handleCount);
                app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));
            }
        SsiVolumeDelete(handleList[0]);
        }
    }
    return 0;

volumeRenameTest:;
    {
        handleCount = 10;

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        sessionHandle = SSI_NULL_HANDLE;
//        result = SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));

        result = SsiGetArrayHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

        if (handleCount)
        {
            if (result == SSI_StatusOk )
            {
                handleCount = 10;

                result = SsiGetVolumeHandles(sessionHandle, SSI_ScopeTypeArray, handleList[0], handleList, &handleCount);
                app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));
            }
            SsiVolumeRename(handleList[0], (SSI_Char *)"NewName");
        }
    }
    return 0;

storagePoolTest:;

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        handleCount = 10;
        result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("%d. Status = %d (%s) handleCount = %d\n", __LINE__, result, status2str(result), handleCount);

        for (i=0; i<handleCount; i++)
    {
                app_log("%d End device handle=0x%x", i, handleList[i]);
        SsiDiskAssignStoragePool(handleList[i], (i%2)+1);
        }

        goto end;

raidHandles:

#define USE_SESSION
        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;

#ifdef USE_SESSION
    result = SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u; Status = %d (%s);", sessionHandle, result, status2str(result));
#endif
    app_log("Count = %d; Status = %d (%s);\n", handleCount, result, status2str(result));

    if (result == SSI_StatusOk)
    {
        for(i = 0; i < handleCount; i++)
        {
            result = SsiGetControllerHandles(sessionHandle, SSI_ScopeTypeRaidInfo,  handleList[i],
                             ctrlList, &ctrlCount);
            app_log("Status = %d (%s); Count = %d\n", result, status2str(result), ctrlCount);
            if (result == SSI_StatusOk)
            {
                SSI_ControllerInfo info;
                for(j = 0; j < ctrlCount; j++)
                {
                    result = SsiGetControllerInfo(SSI_NULL_HANDLE, ctrlList[j], &info);
                    app_log("Status = %d (%s)", result, status2str(result));
                    if (result == SSI_StatusOk)
                    {
                        app_log("Info: %d, %s\n", j, info.controllerName);
                    }
                }
            }
        }

    }
    goto end;

#if 0 /* APW */
arrayAction:;

    {
        SSI_ArrayCreateParams   arrayCreateParams;
        SSI_Char        arrayName[SSI_VOLUME_NAME_LENGTH];

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("Status = %d (%s)\n", result, status2str(result));

        {
//      SSI_EndDeviceInfo info;
            unsigned int i;
            struct raidObject *ro;
            for (i = 0; i < handleCount; i++)
            {
//          SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList[i], &info);
                ro = findRaidObjectByHandle(handleList[i]);
                app_log("Info: %d, %s\n", i, ro->path);
            }
        }
        diskHandleTable[0] = handleList[3];
        diskHandleTable[1] = handleList[2];
        diskHandleTable[2] = handleList[1];
        diskHandleTable[3] = handleList[0];
        arrayCreateParams.disks = diskHandleTable;
        arrayCreateParams.numDisks = 2;

        result = arrayCreate(arrayCreateParams, arrayName);

        app_log("Status = %d (%s)\n", result, status2str(result));
    }

    goto end;
#endif /* APW */
volumeInfo:

    handleCount = 10;

    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;
        result = SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));

    result = SsiGetVolumeHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    goto end;

arrayActionSetWriteCache:;
    {
        SSI_Handle arr[HANDLE_LIST_SIZE];
        SSI_Handle handle;
        SSI_Uint32 arrCnt = HANDLE_LIST_SIZE;

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        result = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, arr, &arrCnt);
        app_log("Status = %d (%s); Count = %d\n", result, status2str(result), arrCnt);

        SsiFinalize();

        handle = 2952790016U;
        result = SsiArraySetWriteCacheState(handle, 0);
        app_log("Status = %d (%s)\n", result, status2str(result));
        return 0;
    }

#if 0 /* APW */
volumeAction:;
    SSI_CreateFromDisksParams createFromDisksParams;

    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Count = %d; Status = %d (%s)\n", handleCount, result, status2str(result));

    {
        SSI_EndDeviceInfo info;
        unsigned int i;
        for (i = 0; i < handleCount; i++)
        {
            result = SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList[i], &info);
            app_log("Status = %d (%s)\n\tInfo: %d, %s\n", result, status2str(result), i, info.serialNo);
        }
    }
    createFromDisksParams.numDisks = 2;

    diskHandleTable[0] = handleList[2];
    diskHandleTable[1] = handleList[1];
    diskHandleTable[2] = handleList[4];
    diskHandleTable[3] = handleList[3];

        createFromDisksParams.disks = diskHandleTable;
        createFromDisksParams.sourceDisk = SSI_NULL_HANDLE;
    createFromDisksParams.stripSize = SSI_StripSize8kB;
    createFromDisksParams.raidLevel = SSI_Raid5;
    createFromDisksParams.sizeInBytes = 160040ULL*1000*1000; // 10GB
    strcpy(createFromDisksParams.volumeName, "testVolume");

    result = SsiVolumeCreateFromDisks(createFromDisksParams);
    app_log("Status = %d (%s)\n", result, status2str(result));

/*  createFromDisksParams.numDisks = 2;

    diskHandleTable[0] = handleList[4];
    diskHandleTable[1] = handleList[3];

        createFromDisksParams.disks = diskHandleTable;
        createFromDisksParams.sourceDisk = SSI_NULL_HANDLE;
    createFromDisksParams.stripSize = SSI_StripSize8kB;
    createFromDisksParams.raidLevel = SSI_Raid0;
    createFromDisksParams.sizeInBytes = 50000;
    strcpy(createFromDisksParams.volumeName, "testVolume2");

    result = SsiVolumeCreateFromDisks(createFromDisksParams);
    app_log("Status = %d (%s)\n", result, status2str(result));*/

    {
        SSI_Handle arr;
        SSI_Uint32 cnt = 1;

        result = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, &arr, &cnt);
        app_log("Count = %d, Status = %d (%s)\n", cnt, result, status2str(result));

        result = arrayDelete(arr);
        app_log("Count = %d, Status = %d (%s)\n", cnt, result, status2str(result));

    }

    goto end;

    createFromDisksParams.numDisks = 3;
        createFromDisksParams.disks = diskHandleTable;

    createFromDisksParams.sourceDisk = SSI_NULL_HANDLE;

        strcpy(createFromDisksParams.volumeName, "vol1");

    createFromDisksParams.stripSize = SSI_StripSize8kB;

    createFromDisksParams.raidLevel = SSI_Raid5;
    createFromDisksParams.sizeInBytes = 50000;

    result = SsiVolumeCreateFromDisks(createFromDisksParams);
    app_log("Status = %d (%s)\n", result, status2str(result));

    goto end;
#endif /* APW */

volumeActionRebuild:;
    {
        SSI_Handle vols[HANDLE_LIST_SIZE];
        SSI_Uint32 volCnt = HANDLE_LIST_SIZE;

        result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        result = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, vols, &volCnt);
        app_log("Status = %d (%s); volumeCoount = %d\n", result, status2str(result), volCnt);

        SsiFinalize();

        result = SsiVolumeRebuild(vols[0], 0);
        app_log("Status = %d (%s)\n", result, status2str(result));
        vols[10] = 2952790016U;
        result = SsiVolumeRebuild(vols[10], 0);
        app_log("Status = %d (%s)\n", result, status2str(result));
        return 0;
    }

raidCapabilityTest:
        app_log("Status = %d (%s)\n", result, status2str(result));
    goto end;
volumePolicy:
    SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));
    getchar();
    //result = SsiVolumeSetCachePolicy(name To Handle("md126"),SSI_CachePolicyOff  );
    app_log("Status = %d (%s)\n", result, status2str(result));
    getchar();
    //result = SsiVolumeSetCachePolicy(name To Handle("md126"), SSI_CachePolicyWriteBack);
    app_log("Status = %d (%s)\n", result, status2str(result));
    goto end;
        // ********** CMD  TEST ***************** //
cmdTest: i=0;
    app_log("*** CMD TEST ***");

    SSI_DiskUnlockInfo unlockInfo;

    unlockInfo.userLevelPassword = 1; //user(1)/master(0)
    strncpy(unlockInfo.diskPassword, "0123456789012345678901234567890123", 32);

    SsiDiskUnlock(handle, &unlockInfo);

    unlockInfo.userLevelPassword = 0; //user(1)/master(0)
    SsiDiskUnlock(handle, &unlockInfo);

    goto end;

#if 0 /* APW */
        // ********** UTILS  TEST ***************** //
utilsTest: i=0;


        app_log("*** UTILS TEST ***");

    setDiskCache(handle, SSI_FALSE);
    setDiskCache(handle, 1);

    //handle = name To Handle("sda");
    app_log("Name=%s, handle=%u %u:%u", "sda", handle, handle>>16, handle&0x00FF);
    //name = handle To Name(handle);
    app_log("Handle=%u name=%s", handle, name);
    free(name);


    //handle = getDiskControllerHandle(name To Handle("sda"));
    //app_log("Controller handle=%u", handle);


    //handle = name To Handle("md127");
        app_log("Name=%s, handle=%u %u:%u", "sda", handle, handle>>16, handle&0x00FF);


    SSI_RaidLevel   raidLevel;

    raidLevel = SSI_Raid5;
    size = volume2Component(20, raidLevel, 3);
    app_log("Size=%u\n", size);


    //sprintf(cmdBuffer, "mdadm -S /dev/md127");
    //app_log("RunCmd %s", cmdBuffer);
    //result = execCmd(cmdBuffer);
    //app_log("Result %d\nOutput:\n%s", result, cmdBuffer);

    //SSI_VolumeDelete(name To Handle("md126"));

    //handle = getDiskArrayHandle(name To Handle("sdd"));
    //name = handle To Name(handle);
    app_log("Array handle = %d  %s", handle, name);
    free(name);
        goto end;

#endif /* APW */

        // ********** CONTROLLER HANDLER TEST ***************** //
controllerTest:

    app_log("*** CONTROLLER HANDLER TEST ***");

    SsiInitialize();

    handleCount  = 30;

    result = SsiGetControllerHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Handle count = %d  result = %d ", handleCount, result);

    for (i=0; i<handleCount; i++) {
                app_log("%d Controller handle=0x%x", i, handleList[i]);

        result = SsiGetControllerInfo(SSI_NULL_HANDLE, handleList[i], &controllerInfo);
                app_log("Status = %d (%s)\n", result, status2str(result));

#if 0 /* APW */
                // display info
                printControllerInfo(&controllerInfo);
#endif /* APW */
        }
        goto end;

testBug:

    app_log("*** CONTROLLER HANDLER TEST ***");

    SsiInitialize();

    handleCount  = 10;

    {
        SSI_Handle tmp;
        result = SsiSessionOpen(&sessionHandle);
        app_log("Result = %d (%s)", result, status2str(result));

        result = SsiSessionOpen(&tmp);
        app_log("Result = %d (%s)", result, status2str(result));

        result = SsiSessionClose(tmp);
        app_log("Result = %d (%s)", result, status2str(result));
    }

    result = SsiSessionClose(sessionHandle);
    app_log("Result = %d (%s)", result, status2str(result));
    sessionHandle = 0x234234;

    sessionHandle = SSI_NULL_HANDLE;

//  result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, NULL, &handleCount);
    //  handleCount--;
    {
        SSI_Handle tmp[10];
        SSI_Uint32 c = 10;
        result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, tmp, &c);
        app_log("Handle count = %d  result = %d (%s)", c, result, status2str(result));
        result = SsiGetControllerHandles(sessionHandle, SSI_ScopeTypeNone, tmp[0], handleList, &handleCount);
    }
    app_log("Handle count = %d  result = %d (%s)", handleCount, result, status2str(result));

//  result = SsiGetControllerInfo(SSI_NULL_HANDLE, 0x234234, &controllerInfo);
//  app_log("Result = %d (%s)", result, status2str(result));

    if (result != SSI_StatusOk)
    {
        app_log("Exit");
        goto end;
    }
//  sessionHandle = 0x234234;

    for (i=0; i<handleCount; i++) {
                app_log("%d Controller handle=0x%x", i, handleList[i]);

        result = SsiGetControllerInfo(sessionHandle, handleList[i], &controllerInfo);
                app_log("Status = %d (%s)\n", result, status2str(result));

#if 0 /* APW */
                // display info
        if (result == SSI_StatusOk)
        {
            printControllerInfo(&controllerInfo);
        }
#endif /* APW */
        }
        goto end;

        // ********** EVENTING TEST ***************** //
eventTest:

    /*log_init(SSI_LOG_DEBUG, "log.txt");*/

        app_log("*** EVENTING TEST ***");
    SsiInitialize();
    app_log(" 1. Get Handle");

    result = SsiGetEventHandle(&handle1);
    app_log("Status = %d Handle = %u\n", result, handle1);

    result = SsiGetEventHandle(&handle2);
    app_log("Status = %d Handle = %u\n", result, handle2);

    app_log(" 2a. Event Wait handle = %u (0x%08X)", handle1);
    result = SsiEventWait(5000, handle1);
    app_log("Status = %d\n", result);

    app_log(" 2b. Event Wait handle = %u (0x%08X)", handle1);
    result = SsiEventWait(0, handle2);
    app_log("Status = %d\n", result);

    app_log(" Free Handle %d", handle2);
    result = SsiFreeEventHandle(handle2);
    app_log("Status = %d\n", result);

    app_log(" Free Handle %d", handle1);
        result = SsiFreeEventHandle(handle1);
    app_log("Status = %d\n", result);

    result = SsiFreeEventHandle(1231231);
    app_log("Status = %d\n", result);

        goto end;

    // ********** EDGE DEVICE HANDLER / INFO TEST ***************** //
enclosureTest: ;
    app_log("*** ENCLOSURE HANDLER / INFO TEST ***");
    result = SsiInitialize();
    handleCount = 8;
    result = SsiGetEnclosureHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);

    for (i=0; i < min(handleListSize, handleCount); i++) {
        app_log("%d Enclosure handle=0x%x", i, handleList[i]);
        SsiGetEnclosureInfo(SSI_NULL_HANDLE, handleList[i], &enclosureObject);
#if 0 /* APW */
        printEnclosureInfo(&enclosureObject);
#endif /* APW */
    }
    goto end;

    // ********** EDGE DEVICE HANDLER / INFO TEST ***************** //
routingDeviceTest: ;
    app_log("*** ROUTING DEVICE HANDLER / INFO TEST ***");
    result = SsiInitialize();
    handleCount = 8;
    result = SsiGetRoutingDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);

    for (i=0; i<min(handleListSize,handleCount); i++) {
        app_log("%d Routing device handle=0x%x", i, handleList[i]);
        SsiGetRoutingDeviceInfo(SSI_NULL_HANDLE, handleList[i], &routingDevObject);
#if 0 /* APW */
        printRoutingDeviceInfo(&routingDevObject);
#endif /* APW */
    }
    goto end;

    // ********** PHY DEVICE HANDLER / INFO TEST ***************** //
phyTest: ;
    app_log("*** PHY HANDLER / INFO TEST ***");
    result = SsiInitialize();
    //SsiSessionOpen(&sessionHandle);
    handleCount = 200;
    result = SsiGetPhyHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);

    for (i=0; i<min(handleListSize,handleCount); i++) {
        app_log("%d Phy handle=0x%x", i, handleList[i]);
        SsiGetPhyInfo(SSI_NULL_HANDLE, handleList[i], &phyObject);
#if 0 /* APW */
        printPhyInfo(&phyObject);
#endif /* APW */
    }
    goto end;

// ********** PHY DEVICE HANDLER / INFO TEST ***************** //
portTest: ;
    app_log("*** PORT HANDLER / INFO TEST ***");
    result = SsiInitialize();
    //SsiSessionOpen(&sessionHandle);
    handleCount = 100;
    result = SsiGetPortHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);
    for (i=0; i<min(handleListSize,handleCount); i++) {
        app_log("%d Port handle=0x%x", i, handleList[i]);
        SsiGetPortInfo(SSI_NULL_HANDLE, handleList[i], &portObject);
#if 0 /* APW */
        printPortInfo(&portObject);
#endif /* APW */
    }
    goto end;

        // ********** END DEVICE HANDLER / INFO TEST ***************** //
endDevInfoTest:
    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;
#ifdef USE_SESSION
    SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u",sessionHandle);
#endif

    EndDeviceInfoTc(sessionHandle);
    goto end;

portInfoTest:
    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;
#ifdef USE_SESSION
    SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u",sessionHandle);
#endif

    PortInfoTc(sessionHandle);
    goto end;

phyInfoTest:
    result = SsiInitialize();
    app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;
#ifdef USE_SESSION
    SsiSessionOpen(&sessionHandle);
    app_log("Session handle = %u",sessionHandle);
#endif

    PhyInfoTc(sessionHandle);
    goto end;

endDeviceTest: ;
    app_log("*** END DEVICE HANDLER / INFO TEST ***");
    result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

        handleCount = 50;
        result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("%d. Status = %d (%s) handleCount = %d\n", __LINE__, result, status2str(result), handleCount);

    for (i=0; i<handleCount; i++) {
                app_log("%d End device handle=0x%x", i, handleList[i]);

                result = SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList[i], &endDeviceInfo);
                app_log("Status = %d (%s)\n", result, status2str(result));

                // display info
            if (result)
        {
            app_log("Can't display info, status = %s\n", status2str(result));
        }
#if 0 /* APW */
        else
        {
            printEndDeviceInfo(&endDeviceInfo);
        }
#endif /* APW */
        }
    return 0;
    // clear metadata
    if (handleCount > 0)
    {
        //result = SsiDiskClearMetadata(handleList[handleCount-1]);
        //app_log("Status = %d (%s)\n", result, status2str(result));
    }

    result = SsiInitialize();
        app_log("Status = %d\n", result);

    result = SsiDiskAssignStoragePool(524288, 1);

    handleListSize=2;
    handleCount=handleListSize;
    handleCount = handleListSize;
    result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
//  result = SsiGetEndDeviceHandles(SSI_NULL_HANDLE, SSI_ScopeTypeArray, 234, handleList, &handleCount);

    app_log("Status = %d (%s)\n", result, status2str(result));
        app_log("handleCount=%d handleListSize=%d", handleCount, handleListSize);

    for (i=0; i<min(handleListSize,handleCount); i++) {
                app_log("%d End device handle=0x%x", i, handleList[i]);
                //app_log("%s", handleToArrayName(127));
        //getTable
                SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList[i], &endDeviceInfo);
#if 0 /* APW */
                // display info
                printEndDeviceInfo(&endDeviceInfo);
#endif /* APW */
        }

    SsiSessionOpen(&sessionHandle);


    goto end;

endDeviceTestScopes:
    app_log("*** END DEVICE HANDLER / INFO TEST ***");
    result = SsiInitialize();
        app_log("Status = %d (%s)\n", result, status2str(result));

    sessionHandle = SSI_NULL_HANDLE;

        handleCount = HANDLE_LIST_SIZE;
        result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("%d. Status = %d (%s) handleCount = %d\n", __LINE__, result, status2str(result), handleCount);

    result = SsiGetControllerHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, ctrlList, &ctrlCount);
    app_log("%d. Status = %d (%s) ctrlCount = %d\n", __LINE__, result, status2str(result), ctrlCount);

    for (i = 0; i < ctrlCount; i++)
    {
                app_log("%d Controller handle=0x%x", i, ctrlList[i]);

        result = SsiGetEndDeviceHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("%d. Status = %d (%s) handleCount = %d\n", __LINE__, result, status2str(result), handleCount);

                // display info
        for (j = 0; j < handleCount; j++)
        {
            app_log("%d End device handle=0x%x", j, handleList[j]);

            result = SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList[j], &endDeviceInfo);
            if (result)
            {
                app_log("Can't display info, status = %s\n", status2str(result));
            }
#if 0 /* APW */
            else
            {
                printEndDeviceInfo(&endDeviceInfo);
            }
#endif /* APW */
        }
        }
    return 0;
        // ********** ARRAY HANDLER / INFO TEST ***************** //
volumeTest: ;

    SsiInitialize();

    sessionHandle = SSI_NULL_HANDLE;
        result = SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u, Status = %d (%s)\n", sessionHandle, result, status2str(result));

    result = SsiGetVolumeHandles(sessionHandle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
        app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);

        for (i=0; i<handleCount; i++) {
                app_log("%d Volume handle=0x%x", i, handleList[i]);

                result = SsiGetVolumeInfo( sessionHandle, handleList[i], &volumeInfo);
                app_log("Status = %d (%s)\n", result, status2str(result));

        if (result)
        {
            app_log("Can't display info, status = %s\n", status2str(result));
        }
#if 0 /* APW */
        else
        {
            // display info
            printVolumeInfo(&volumeInfo);
        }
#endif /* APW */
        }
        goto end;

volumeTest2: ;

    SsiInitialize();

    {
        SSI_Handle arrayHandle;
        handleCount = 1;
        result = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, &arrayHandle, &handleCount);
        app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);
        handleCount = 10;
        result = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeArray, arrayHandle, handleList, &handleCount);
        app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);

        for (i=0; i<handleCount; i++) {
            app_log("%d Volume handle=0x%x", i, handleList[i]);

            result = SsiGetVolumeInfo( SSI_NULL_HANDLE, handleList[i], &volumeInfo);
            app_log("Status = %d (%s)\n", result, status2str(result));

#if 0 /* APW */
            // display info
            printVolumeInfo(&volumeInfo);
#endif /* APW */
        }
    }
        goto end;


    // ********** ARRAY HANDLER / INFO TEST ***************** //
arrayTest: ;

    SsiInitialize();

    result = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
    app_log("Status = %d (%s) handleCount = %d\n", result, status2str(result), handleCount);

    for (i=0; i<handleCount; i++) {
        app_log("%d Array handle=0x%x", i, handleList[i]);

        result = SsiGetArrayInfo( SSI_NULL_HANDLE, handleList[i], &arrayInfo);
        app_log("Status = %d (%s)\n", result, status2str(result));

#if 0 /* APW */
        // display info
        printArrayInfo(&arrayInfo);
#endif /* APW */
    }
    goto end;
    // ********** SESSION HANDLER TEST ***************** //
sessionTest:
    SsiInitialize();

    SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u",sessionHandle);

    SsiInitialize();

        SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u",sessionHandle);

    SsiSessionOpen(&sessionHandle);
        app_log("Session handle = %u",sessionHandle);


    goto end;

sessionTest2:

    sessionTest();

        //printSessions();

end:
    i = 0;
    return 0;
}

