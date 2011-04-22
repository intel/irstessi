// ex: tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab:

#include <iostream>
#include <ssi.h>

using namespace std;

/* */
#define INITIAL_COUNT	96

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Handle handle;
	SSI_Handle handleList[INITIAL_COUNT];
	SSI_Handle handleList1[INITIAL_COUNT];
	unsigned int handleCount;
	unsigned int handleCount1;
	
    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        cout << "E: unable to initialize the SSI library (errcode:" << status << ")" << endl;
        return -1;
    }
    status = SsiSessionOpen(&handle);
    if (status == SSI_StatusOk) {
		handleCount = INITIAL_COUNT;
		status = SsiGetControllerHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << handleCount << "\tControllers: " << endl;
		} else {
			cout << "E: Unable to retrieve Controller handles." << endl;
		}
										
		for(unsigned i = 0; i < handleCount; i++ ) {
			//****** End Devices ************
			cout << "Controller " << i << endl;
			handleCount1 = INITIAL_COUNT;
			status = SsiGetEndDeviceHandles(handle, SSI_ScopeTypeControllerDirect, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
		
				cout << handleCount1 << "\tEndDevices Direct" << endl;
			} else {
				cout << "E: Unable to retrieve EndDevice handles." << endl;
			}
			handleCount1 = INITIAL_COUNT;
			status = SsiGetEndDeviceHandles(handle, SSI_ScopeTypeControllerAll, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout << handleCount1 << "\tEndDevices All" << endl;
			} else {
				cout << "E: Unable to retrieve EndDevice handles." << endl;
			}
			
			SSI_EndDeviceInfo info;
			for (unsigned int j = 0; j < handleCount1; j++) {
				status = SsiGetEndDeviceInfo(SSI_NULL_HANDLE, handleList1[j], &info);
				if (status == SSI_StatusOk) {
					cout << "Handle=0x" << hex << info.endDeviceHandle << dec << endl;
				
				} else {
					cout << "E: Unable to retrieve disk info" << endl;
				}
			}
			// ****** Phys *******************
			handleCount1 = INITIAL_COUNT;
			status = SsiGetPhyHandles(handle, SSI_ScopeTypeControllerDirect, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout << handleCount1 << "\tPhys Direct"<< endl;
			} else {
				cout << "E: Unable to retrieve Phy handles." << endl;
			}		
			handleCount1 = INITIAL_COUNT;
			status = SsiGetPhyHandles(handle, SSI_ScopeTypeControllerAll, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout << handleCount1 << "\tPhys All"<< endl;
			} else {
				cout << "E: Unable to retrieve Phy handles." << endl;
			}		
			//****** Ports ****************
			handleCount1 = INITIAL_COUNT;
			status = SsiGetPortHandles(handle, SSI_ScopeTypeControllerDirect, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout << handleCount1 << "\tPorts Direct" << endl;
			} else {
				cout << "E: Unable to retrieve Port handles." << endl;
			}
			handleCount1 = INITIAL_COUNT;
			status = SsiGetPortHandles(handle, SSI_ScopeTypeControllerAll, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout << handleCount1 << "\tPorts All" << endl;
			} else {
				cout << "E: Unable to retrieve Port handles." << endl;
			}			
		}		
		
		handleCount1 = INITIAL_COUNT;
		status = SsiGetPhyHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList1, &handleCount1);
		if (status == SSI_StatusOk) {
			cout << endl << handleCount1 << "\tPhys total " << endl;
		} else {
			cout << "E: Unable to retrieve Phy handles." << endl;
		}
		handleCount1 = INITIAL_COUNT;
		status = SsiGetPortHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList1, &handleCount1);
		if (status == SSI_StatusOk) {
			cout << handleCount1 << "\tPorts total " << endl;
		} else {
			cout << "E: Unable to retrieve Port handles." << endl;
		}
		//******** Routing Device ***********
		handleCount = INITIAL_COUNT;
		status = SsiGetRoutingDeviceHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << handleCount << "\tRoutingDevices " << endl;
		} else {
			cout << "E: Unable to retrieve RoutingDevice handles." << endl;
		}
		
		for(unsigned i = 0; i < handleCount; i++ ) {
			cout << "Routing Device " << i << endl;
			cout << "Handle=0x" << hex << handleList[i] << dec << endl;
			//****** EndDevices ***************
			handleCount1 = INITIAL_COUNT;
			status = SsiGetEndDeviceHandles(handle, SSI_ScopeTypeRoutingDevice, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				
				cout << handleCount1 << "\tEndDevices" << endl;
			} else {
				cout << "E: Unable to retrieve EndDevice handles." << endl;
			}
			// ****** Phys *******************
			handleCount1 = INITIAL_COUNT;
			status = SsiGetPhyHandles(handle, SSI_ScopeTypeRoutingDevice, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout  << handleCount1 << "\tPhys" << endl;
			} else {
				cout << "E: Unable to retrieve Phy handles." << endl;
			}
			//****** Ports ****************
			handleCount1 = INITIAL_COUNT;
			status = SsiGetPortHandles(handle, SSI_ScopeTypeRoutingDevice, handleList[i], handleList1, &handleCount1);
			if (status == SSI_StatusOk) {
				cout << handleCount1 << "\tPorts" << endl;
			} else {
				cout << "E: Unable to retrieve Port handles." << endl;
			}
		}

		handleCount = INITIAL_COUNT;
		status = SsiGetEnclosureHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "Exnclosures: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve Enclosure handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetArrayHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "Arrays: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve Array handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetVolumeHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "Volumes: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve Volume handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetRaidInfoHandles(handle, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "RaidInfos: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve RaidInfo handles." << endl;
		}
    } else {
        cout << "E: unable to open a session (errcode:" << status << ")" << endl;
    }
    status = SsiFinalize();
    if (status != SSI_StatusOk) {
        cout << "E: unable to finalize the SSI library (errcode:" << status << ")" << endl;
    }
    return 0;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
