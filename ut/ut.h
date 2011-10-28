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
#include <string>

#define HANDLE_COUNT 100
#define MAX_COUNT 200

enum TestResult {
    NotRun = -1,
    Passed = 0,
    Failed = 1
};

using namespace std;

string statusStr[] = {"SSI_StatusOk",               "SSI_StatusInsufficientResources",    "SSI_StatusInvalidParameter",       "SSI_StatusInvalidHandle",     "SSI_StatusInvalidStripSize",
                      "SSI_StatusInvalidString",    "SSI_StatusInvalidSize",              "SSI_StatusInvalidRaidLevel",       "SSI_StatusInvalidSession",    "SSI_StatusTimeout",
                      "SSI_StatusNotImplemented",   "SSI_StatusDuplicate",                "SSI_StatusNotInitialized",         "SSI_StatusBufferTooSmall",    "SSI_StatusNotSupported",
                      "SSI_StatusFailed",           "SSI_StatusInternalError",            "SSI_StatusInvalidScope",           "SSI_StatusInvalidState",      "SSI_StatusRetriesExceeded",
                      "SSI_StatusWrongPassword",    "SSI_StatusDataExceedsLimits",        "SSI_StatusInsufficientPrivileges", "SSI_StatusDriverBusy" };

string devtype[] = {"SSI_DeviceTypeUnknown", "SSI_DeviceTypeController", "SSI_DeviceTypeEndDevice", "SSI_DeviceTypeRoutingDevice"};
string physpeed[] = {"SSI_PhySpeedUnknown" , "SSI_PhySpeed_1_5_GEN1", "SSI_PhySpeed_3_0_GEN2", "SSI_PhySpeed_6_0_GEN3"};
string phyprotocol[] = {"SSI_PhyProtocolUnknown", "SSI_PhyProtocolSATA", "SSI_PhyProtocolSMP", "SSI_PhyProtocolSTP", "SSI_PhyProtocolSSP"};
string diskstate[] = {"SSI_DiskStateUnknown", "SSI_DiskStateOffline", "SSI_DiskStateMissing", "SSI_DiskStateFailed", "SSI_DiskStateSmartEventTriggered", "SSI_DiskStateConfigIsUprev", "SSI_DiskStateNormal", "SSI_DiskStateLocked"};
string diskusage[] = {"SSI_DiskUsageUnknown", "SSI_DiskUsageArrayMember", "SSI_DiskUsagePassThru", "SSI_DiskUsageOfflineArray", "SSI_DiskUsageSpare", "SSI_DiskUsageArrayMemberReadOnlyMount", "SSI_DiskUsagePassThruReadOnlyMount"};


