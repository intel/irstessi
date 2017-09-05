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
    SSI_Handle handles[HANDLE_COUNT];

    (void)argc;
    (void)argv;

    if (system("cd scripts; ./clean.sh") != 0) {
        cout << "E: unable to stop existing md devices and clean metadata." << endl;
        return -1;
    }
    if (system("sleep 5; cd scripts; ./create.sh ahci 1 64 1000000 2 ") != 0) {
        cout << "E: unable to create" << endl;
        return -1;
    }

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }

    /* get handles of volumes */
    cout << "-->SsiGetVolumeHandles..." << endl;
    count = HANDLE_COUNT;
    status = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "volumes: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << dec <<endl;
        }
    } else {
        cout << "E: unable to get volume handles (status=" << status << ")" << endl;
    }

    status = SsiVolumeInitialize(*handles);
    if (status == SSI_StatusOk) {
        cout << "volume initialized " << endl;
    } else {
        cout << "E: unable to initialize volume (status=" << status << ")" << endl;
    }

    SSI_Handle session;

    if (system("cd scripts; ./clean.sh") != 0) {
        cout << "E: unable to stop existing md devices and clean metadata." << endl;
        return -1;
    }
    if (system("sleep 5; cd scripts; ./create.sh ahci 1 64 1000000 2 ") != 0) {
        cout << "E: unable to create 2" << endl;
        return -1;
    }

    status = SsiSessionOpen(&session);
    if (status == SSI_StatusOk) {
        cout << "Session\tHandle = 0x" << hex << session << dec << endl;
    } else {
        cout << "E: unable to open session (status=" << status << ")" << endl;
    }

    /* get handles of volumes */
    cout << "-->SsiGetVolumeHandles..." << endl;
    count = HANDLE_COUNT;
    status = SsiGetVolumeHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
    if (status == SSI_StatusOk) {
        cout << "volumes: " << count << endl;
        for (unsigned int i = 0; i < count; ++i) {
            cout << "\thandle=0x" << hex << handles[i] << dec <<endl;
        }
    } else {
        cout << "E: unable to get volume handles (status=" << status << ")" << endl;
    }

    status = SsiVolumeInitialize(*handles);
    if (status == SSI_StatusOk) {
        cout << "volume initialized " << endl;
    } else {
        cout << "E: unable to initialize volume (status=" << status << ")" << endl;
    }

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
