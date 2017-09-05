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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <ssi.h>
#define HANDLE_COUNT 100

using namespace std;

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Uint32 count;

    (void)argc;
    (void)argv;

    if (system("cd scripts; ./clean.sh") != 0) {
        cout << "E: unable to stop existing md devices and clean metadata." << endl;
        return -1;
    }

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }

    SSI_Handle session;
    SSI_Handle handles[HANDLE_COUNT];

    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to open session" << endl;
        return -1;
    }
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
                    cout << "\tdevice suitable for raid"<< endl;
                } else {
                    cout << "\tdevice unusable" << endl;
                }
            }
        }
    }

    if (count < 1) {
        cout << "E: there are no available devices in the system to create a spare." << endl;
        return -2;
    }

    for (unsigned int i = 0; i < count; i++) {
        cout << "-->Adding spare " << i << " (SsiDiskMarkAsSpare)..." << endl;
        /* add a spare */
        status = SsiDiskMarkAsSpare(handles[i], SSI_NULL_HANDLE);
        if (status == SSI_StatusOk) {
            cout << "Added disk 0x"<< hex << handles[i] << dec << endl;
        } else {
            cout << "E: unable to add disk to array (status=" << status << ")" << endl;
        }
        system("sleep 2; cat /proc/mdstat");
    }
    for (unsigned int i = 0; i < count; i++) {
        status = SsiDiskUnmarkAsSpare(handles[i]);
        if (status == SSI_StatusOk) {
            cout << "Removed spare " << hex << handles[i] << dec << endl;
        } else {
            cout << "E: unable to remove disk from array (status=" << status << ")" << endl;
        }
    }

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
