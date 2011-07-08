
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

    (void)argc;
    (void)argv;

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        return -1;
    }

    SSI_Handle handle, session;

    status = SsiSessionOpen(&session);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to open session" << endl;
        return -1;
    }

    status = SsiGetEventHandle(&handle);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to get event handle" << endl;
        return -1;
    }
    cout << "handle = 0x" << hex << handle << dec << endl;

    for (int i = 0; i < 5; i++) {
        cout << "Waiting for event [" << i << "]...";
        status = SsiEventWait(5*1000, handle);
        if (status != SSI_StatusOk)
            cout << "\ttimed out" << endl;
        else
            cout << "\tsucceded" << endl;
    }

    status = SsiFreeEventHandle(handle);
    if (status != SSI_StatusOk) {
        cout << "E. Unable to free event handle" << endl;
    } else {
        cout << "handle freed " << endl;
    }

    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        return -2;
    }
    return 0;
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
