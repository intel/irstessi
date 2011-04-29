
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

using namespace std;

int main(int argc, char *argv[])
{
	SSI_Status status;
	SSI_Uint32 count;
	SSI_Handle volumeHandle;
	SSI_VolumeInfo volumeInfo;
	
	(void)argc;
	(void)argv;

	
	status = SsiInitialize();
	if (status != SSI_StatusOk) {
		return -1;
	}

	SSI_Handle r1_disks[2];
	SSI_Handle handles[10];
	unsigned int j = 0;

	count = 10;
	status = SsiGetVolumeHandles(SSI_NULL_HANDLE, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handles, &count);
	if (status == SSI_StatusOk) {
		cout << "volumes: " << count << endl;
		for (unsigned int i = 0; i < count; ++i) {
			cout << "\thandle=0x" << hex << handles[i] << endl;
		}
	} else {
		cout << "E: unable to get volume handles (status=" << status << ")" << endl;
	}

	if (count > 0) {
		status = SsiGetVolumeInfo(SSI_NULL_HANDLE, handles[0], &volumeInfo);
		if (status == SSI_StatusOk) {
			cout << "\thandle=0x" << hex << volumeInfo.volumeHandle<< endl;
			/** Array handle */
			cout << "\thandle=0x" << hex << volumeInfo.arrayHandle<< endl;
			/** Ordinal number (index) of volume in array */
			cout << "\tarrayOrdinal" << volumeInfo.arrayOrdinal << endl;
			/** Volume name */
			cout << "\tvolumeName" << volumeInfo.volumeName << endl;
			/** Volume state */
			cout << "volume state: " << volumeInfo.state << endl;
			/** Volume's RAID level */
			cout << "volume raidLevel: " << volumeInfo.raidLevel << endl;
		} else {
			cout << "E: unable to get volume info (status=" << status << ")" << endl;
		}
	}

	status = SsiFinalize();
	if (status != SSI_StatusOk) {
		return -2;
	}
	return 0; 
}

// ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=96 noexpandtab:
