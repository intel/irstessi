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
