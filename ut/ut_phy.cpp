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

#include <ssi.h>
#include "ut.h"

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Handle session;
    SSI_Handle handle[HANDLE_COUNT];
    SSI_Uint32 count;

    (void)argc;
    (void)argv;

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }
    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        return -1;
    }

    /* get devices */
    cout << "-->SsiGetPhyHandles ";
    count = HANDLE_COUNT;
    status = SsiGetPhyHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handle, &count);
    if (status == SSI_StatusOk) {
        SSI_PhyInfo info;
        cout << count << endl;
        cout << "Turn leds on..." << endl;
        for (unsigned int i = 0; i < count; i++) {
            cout << "handle " << i << "\t"<< hex << handle[i] << dec;
            status = SsiGetPhyInfo(session, handle[i], &info);
            if (status == SSI_StatusOk) {
                status = SsiPhyLocate(handle[i], true);
                if (info.deviceType == SSI_DeviceTypeEndDevice) {
                    cout << "\tEndDevice phy: status = " << statusStr[status] << endl;
                } else {
                    cout << "\tOther phy: status = "<< statusStr[status] << " dev type: " << devtype[info.deviceType] << endl;
                }
            } else {
                cout << "E: unable to get phy info (status=" << statusStr[status] << ")" << endl;
            }
        }
        usleep(9000000);
        cout << "Turn leds off..." << endl;
        for (unsigned int i = 0; i < count; i++) {
            cout << "handle " << i << "\t"<< hex << handle[i] << dec;
            status = SsiGetPhyInfo(session, handle[i], &info);
            if (status == SSI_StatusOk) {
                status = SsiPhyLocate(handle[i], false);
                if (info.deviceType == SSI_DeviceTypeEndDevice) {
                    cout << "\tEndDevice phy: status = " << statusStr[status] << endl;
                } else {
                    cout << "\tOther phy: status = "<< statusStr[status] << " dev type: " << devtype[info.deviceType] << endl;
                }
            } else {
                cout << "E: unable to get phy info (status=" << statusStr[status] << ")" << endl;
            }
        }
    } else {
        cout << "E: unable to get phy handles (status=" << statusStr[status] << ")" << endl;
    }

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
