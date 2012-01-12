
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ssi.h>
#include "ut.h"

TestResult mark_as_spare(SSI_Handle *endDevices, unsigned int count);
TestResult create_and_delete(SSI_Handle *endDevices, unsigned int count);
TestResult create_delete_and_create(SSI_Handle *endDevices, unsigned int count);
TestResult create_and_rename(SSI_Handle *endDevices, unsigned int count);
TestResult create_two_volumes(SSI_Handle *endDevices, unsigned int count);
TestResult expand_volume(SSI_Handle *endDevices, unsigned int count);
int clean();

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Uint32 count;
    unsigned int passed = 0;
    unsigned int failed = 0;
    unsigned int notrun = 0;
    TestResult rv;
    SSI_Handle session;

    (void)argc;
    (void)argv;

    if (clean() != 0)
        return -1;

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }
    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        return -1;
    }
    SSI_Handle handles[HANDLE_COUNT];
    SSI_Handle endDevices[HANDLE_COUNT];
    unsigned int j = 0;

    /* get devices */
    cout << "-->SsiGetEndDeviceHandles"<< endl;
    count = HANDLE_COUNT;
    status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        SSI_EndDeviceInfo info;
        cout << count << endl;
        for (unsigned int i = 0; i < count; i++) {
            cout << "handle " << i << "\t"<< hex << handles[i] << dec;
            status = SsiGetEndDeviceInfo(session, handles[i], &info);
            if (status == SSI_StatusOk) {
                if (info.systemDisk == SSI_FALSE && info.deviceType == SSI_EndDeviceTypeDisk &&
                    info.state == SSI_DiskStateNormal && info.usage == SSI_DiskUsagePassThru) {
                    endDevices[j++] = handles[i];
                    cout << "\tdevice suitable for raid"<< endl;
                } else {
                    cout << "\tdevice unusable" << endl;
                }
            }
        }
    } else {
        cout << "E: unable to get end device handles (status=" << statusStr[status] << ")" << endl;
    }

    if (j < 10) {
        cout << "E: there are no available devices in the system to create a RAID." << endl;
        return -2;
    }

    cout << "Usable disks:" << endl;
    for (unsigned int i = 0; i < j; i++) {
        cout << "Disk " << i << "\t"<< hex << endDevices[i] << dec << endl;
    }

    cout << "Test 0: Mark as spare" << endl;
    rv = mark_as_spare(endDevices, j);
    switch (rv) {
        case Passed:
            cout << "Passed" << endl;
            passed++;
            break;
        case NotRun:
            cout << "NotRun" << endl;
            notrun++;
            break;
        default:
            cout << "Failed" << endl;
            failed++;
    }

    cout << "Test 1: Create and Delete" << endl;
    rv = create_and_delete(endDevices, j);
    switch (rv) {
        case Passed:
            cout << "Passed" << endl;
            passed++;
            break;
        case NotRun:
            cout << "NotRun" << endl;
            notrun++;
            break;
        default:
            cout << "Failed" << endl;
            failed++;
    }

    cout << "Test 2: Create, Delete and Create" << endl;
    rv = create_delete_and_create(endDevices, j);
    switch (rv) {
        case Passed:
            cout << "Passed" << endl;
            passed++;
            break;
        case NotRun:
            cout << "NotRun" << endl;
            notrun++;
            break;
        default:
            cout << "Failed" << endl;
            failed++;
    }

    cout << "Test 3: Create and Rename" << endl;
    rv = create_and_rename(endDevices, j);
    switch (rv) {
        case Passed:
            cout << "Passed" << endl;
            passed++;
            break;
        case NotRun:
            cout << "NotRun" << endl;
            notrun++;
            break;
        default:
            cout << "Failed" << endl;
            failed++;
    }

    cout << "Test 4: Create 2 volumes" << endl;
    rv = create_two_volumes(endDevices, j);
    switch (rv) {
        case Passed:
            cout << "Passed" << endl;
            passed++;
            break;
        case NotRun:
            cout << "NotRun" << endl;
            notrun++;
            break;
        default:
            cout << "Failed" << endl;
            failed++;
    }

    cout << "Test 5: Create and Expand" << endl;
    rv = expand_volume(endDevices, j);
    switch (rv) {
        case Passed:
            cout << "Passed" << endl;
            passed++;
            break;
        case NotRun:
            cout << "NotRun" << endl;
            notrun++;
            break;
        default:
            cout << "Failed" << endl;
            failed++;
    }

    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Not run: " << notrun << endl;

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

int clean()
{
    if (system("cd scripts; ./clean.sh") != 0) {
        cout << "E: unable to stop existing md devices and clean metadata." << endl;
        return 1;
    }
    return 0;
}


SSI_Status create(SSI_Handle *endDevices, unsigned int count, SSI_Handle *volumeHandle)
{

    SSI_Status status;
    /* create raid0 volume*/
    SSI_CreateFromDisksParams params;

    if (endDevices == 0 || count == 0 || volumeHandle == 0 )
        return SSI_StatusFailed;
    params.disks = endDevices;
    params.numDisks = 3;
    params.sourceDisk = SSI_NULL_HANDLE;
    strcpy(params.volumeName, "ut_Volume");
    params.stripSize = SSI_StripSize64kB;
    params.raidLevel = SSI_Raid0;
    params.sizeInBytes = 1024UL*1024*1024*2;

    cout << "-->SsiVolumeCreateFromDisks..." << endl;
    status = SsiVolumeCreateFromDisks(params, volumeHandle);
    if (status == SSI_StatusOk) {
        cout << "Created volume: volumeHandle=0x" << hex << *volumeHandle << endl;
    } else {
        cout << "E: Volume creation failed (status=" << statusStr[status] << ")"  << endl;
    }
    usleep(3000000);
    return status;
}

TestResult mark_as_spare(SSI_Handle *endDevices, unsigned int count)
{
    SSI_Handle volumeHandle;
    SSI_Status status;
    status = create(endDevices, count, &volumeHandle);
    if (status != SSI_StatusOk)
        return NotRun;

    cout << "-->SsiDiskMarkAsSpare..." << endl;
    status = SsiDiskMarkAsSpare(endDevices[0], SSI_NULL_HANDLE);
    if (status == SSI_StatusOk) {
        cout << "E. Disk marked as spare" << endl;
        clean();
        return Failed;
    } else {
        cout << "Unable to mark disk as spare (status=" << statusStr[status] << ")" << endl;
        clean();
        return (status == SSI_StatusInvalidState)?Passed:Failed;
    }
}

TestResult create_and_delete(SSI_Handle *endDevices, unsigned int count)
{
    SSI_Handle volumeHandle;
    SSI_Status status;
    status = create(endDevices, count, &volumeHandle);
    if (status != SSI_StatusOk)
        return NotRun;
    /* now delete it */
    cout << "-->SsiVolumeDelete..." << endl;
    status = SsiVolumeDelete(volumeHandle);
    if (status != SSI_StatusOk) {
        cout << "E: unable to delete volume (status=" << statusStr[status] << ")" << endl;
        clean();
        return Failed;
    } else {
        cout << "Volume deleted" << endl;
    }
    return Passed;
}

TestResult create_delete_and_create(SSI_Handle *endDevices, unsigned int count)
{
    SSI_Handle volumeHandle;
    SSI_Status status;

    TestResult rv = create_and_delete(endDevices, count);
    if (rv != Passed)
        return NotRun;

    status = create(endDevices, count, &volumeHandle);
    rv = (status != SSI_StatusOk)?Failed:Passed;
    clean();
    return rv;
}

TestResult create_and_rename(SSI_Handle *endDevices, unsigned int count)
{
    SSI_Handle volumeHandle;
    SSI_Status status;
    TestResult rv = Passed;
    status = create(endDevices, count, &volumeHandle);
    if (status != SSI_StatusOk)
        return NotRun;
    usleep(5000000);
    cout << "-->SsiVolumeRename..." << endl;
    status = SsiVolumeRename(volumeHandle, "ut_Renamed");
    if (status != SSI_StatusOk) {
        cout << "E: unable to rename volume (status=" << statusStr[status] << ")" << endl;
        rv = Failed;
    } else {
        /* check if properly renamed */
        usleep(3000000);
        if (system("grep ut_Renamed /dev/.mdadm/map") == 0)
            cout << "Volume renamed" << endl;
        else
            rv = Failed;
    }
    clean();
    return rv;
}

TestResult create_two_volumes(SSI_Handle *endDevices, unsigned int ecount)
{
    SSI_Handle volumeHandle, arrayHandle;
    SSI_Status status;
    unsigned int count;
    SSI_Handle handles[HANDLE_COUNT];

    status = create(endDevices, ecount, &volumeHandle);
    if (status != SSI_StatusOk)
        return NotRun;

    /* find array handle */
    cout << "-->SsiGetArrayHandles..." << endl;
    count = HANDLE_COUNT;
    status = SsiGetArrayHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "arrays: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << endl;
        }
        if (count > 0) {
            arrayHandle = handles[0];
        }
    } else {
        cout << "E: unable to get array handles (status=" << statusStr[status] << ")" << endl;
        return NotRun;
    }

    /* create raid0 volume on the same array */
    cout << "-->SsiVolumeCreateFromArray..." << endl;
    SSI_CreateFromArrayParams array_params;
    array_params.arrayHandle = arrayHandle;
    strcpy(array_params.volumeName, "ut_Volume_r0_01");
    array_params.stripSize = SSI_StripSize64kB;
    array_params.raidLevel = SSI_Raid0;
    array_params.sizeInBytes = 0;

    status = SsiVolumeCreate(array_params);
    if (status != SSI_StatusOk) {
        cout << "E: unable to create second volume in the same array.(status=" << statusStr[status] << ")" << endl;
        return Failed;
    }

    /*get handles of both volumes */
    cout << "-->SsiGetVolumeHandles..." << endl;
    count = HANDLE_COUNT;
    status = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "volumes: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << dec <<endl;
        }
    } else {
        cout << "E: unable to get volume handles (status=" << statusStr[status] << ")" << endl;
        return Failed;
    }
    clean();
    return Passed;
}

TestResult expand_volume(SSI_Handle *endDevices, unsigned int count)
{
    SSI_Handle volumeHandle;
    SSI_Status status;
    TestResult rv;
    status = create(endDevices, count, &volumeHandle);
    if (status != SSI_StatusOk)
        return NotRun;
    system("mdadm --wait /dev/md/ut_Volume");
    status = SsiExpandVolume(volumeHandle, 0);
    if (status == SSI_StatusOk) {
        /* TODO check needed */
        cout << "Volume expanded" << endl;
        rv = Passed;
    } else {
        cout << "E: unable to expand volume (status=" << statusStr[status] << ")" << endl;
        rv = Failed;
    }
    clean();
    return rv;
}
// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
