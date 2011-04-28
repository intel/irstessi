// ex: tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab:

#include <iostream>
#include <ssi.h>

using namespace std;
#define INITIAL_COUNT	200

int ShowHandles(SSI_Handle handleList[], unsigned int handleCount)
{
	for (unsigned int i = 0; i < handleCount; i++)
			cout << "Handle["<< i << "] = 0x" << hex << handleList[i] << dec << endl;
	return 0;
}

int ControllerScopeStats(SSI_Handle session, SSI_Handle handle)
{
	SSI_Handle handleList[INITIAL_COUNT];
	unsigned int handleCount;
	SSI_Status status;

	cout << "Controller Handle = 0x" << hex << handle << dec << endl;
	
	// ******** End devices *************
	handleCount = INITIAL_COUNT;
	status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeControllerDirect, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tEndDevices Direct" << endl;
	} else {
		cout << "E: Unable to retrieve EndDevice handles." << endl;
	}
	handleCount = INITIAL_COUNT;
	status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeControllerAll, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tEndDevices All" << endl;
	} else {
		cout << "E: Unable to retrieve EndDevice handles." << endl;
	}
	ShowHandles(handleList, handleCount);
	
	// ****** Phys *******************
	handleCount = INITIAL_COUNT;
	status = SsiGetPhyHandles(session, SSI_ScopeTypeControllerDirect, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPhys Direct"<< endl;
	} else {
		cout << "E: Unable to retrieve Phy handles." << endl;
	}		
	handleCount = INITIAL_COUNT;
	status = SsiGetPhyHandles(session, SSI_ScopeTypeControllerAll, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPhys All"<< endl;
	} else {
		cout << "E: Unable to retrieve Phy handles." << endl;
	}		
	//****** Ports ****************
	handleCount = INITIAL_COUNT;
	status = SsiGetPortHandles(session, SSI_ScopeTypeControllerDirect, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPorts Direct" << endl;
	} else {
		cout << "E: Unable to retrieve Port handles." << endl;
	}
	handleCount = INITIAL_COUNT;
	status = SsiGetPortHandles(session, SSI_ScopeTypeControllerAll, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPorts All" << endl;
	} else {
		cout << "E: Unable to retrieve Port handles." << endl;
	}
	return 0;
}

int RoutingDeviceScopeStats(SSI_Handle session, SSI_Handle handle)
{
	SSI_Handle handleList[INITIAL_COUNT];
	unsigned int handleCount;
	SSI_Status status;

	cout << "Routing Device Handle = 0x" << hex << handle << dec << endl;
	//****** EndDevices ***************
	handleCount = INITIAL_COUNT;
	status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeRoutingDevice, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tEndDevices" << endl;
	} else {
		cout << "E: Unable to retrieve EndDevice handles." << endl;
	}
	// ****** Phys *******************
	handleCount = INITIAL_COUNT;
	status = SsiGetPhyHandles(session, SSI_ScopeTypeRoutingDevice, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout  << handleCount << "\tPhys" << endl;
	} else {
		cout << "E: Unable to retrieve Phy handles." << endl;
	}
	//****** Ports ****************
	handleCount = INITIAL_COUNT;
	status = SsiGetPortHandles(session, SSI_ScopeTypeRoutingDevice, handle, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPorts" << endl;
	} else {
		cout << "E: Unable to retrieve Port handles." << endl;
	}
	return 0;
}

int SessionStats(SSI_Handle session)
{
	SSI_Handle handleList[INITIAL_COUNT];
	unsigned int handleCount;
	SSI_Status status;

	// ******** Controllers **************
	handleCount = INITIAL_COUNT;
	status = SsiGetControllerHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tControllers total " << endl;
		for (unsigned int i = 0; i < handleCount; i++)
			ControllerScopeStats(session, handleList[i]);

	} else {
		cout << "E: Unable to retrieve Controller handles." << endl;
	}
	
	// ******** End Devices **************
	handleCount = INITIAL_COUNT;
	status = SsiGetEndDeviceHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tEndDevices total " << endl;
	} else {
		cout << "E: Unable to retrieve End Device handles." << endl;
	}
	
	// ********** Phys ******************
	handleCount = INITIAL_COUNT;
	status = SsiGetPhyHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPhys total " << endl;
	} else {
		cout << "E: Unable to retrieve Phy handles." << endl;
	}
	
	// ********** Ports ****************
	handleCount = INITIAL_COUNT;
	status = SsiGetPortHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tPorts total " << endl;
	} else {
		cout << "E: Unable to retrieve Port handles." << endl;
	}
	
	//******** Routing Devices ***********
	handleCount = INITIAL_COUNT;
	status = SsiGetRoutingDeviceHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << handleCount << "\tRoutingDevices " << endl;
	} else {
		cout << "E: Unable to retrieve RoutingDevice handles." << endl;
	}
	ShowHandles(handleList, handleCount);
	
	// ******* Enclosures *************
	handleCount = INITIAL_COUNT;
	status = SsiGetEnclosureHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << "Exnclosures: " << handleCount << endl;
	} else {
		cout << "E: Unable to retrieve Enclosure handles." << endl;
	}
	
	// ********** Arrays *************
	handleCount = INITIAL_COUNT;
	status = SsiGetArrayHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << "Arrays: " << handleCount << endl;
	} else {
		cout << "E: Unable to retrieve Array handles." << endl;
	}
	
	// ********** Volumes ************
	handleCount = INITIAL_COUNT;
	status = SsiGetVolumeHandles(session, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << "Volumes: " << handleCount << endl;
	} else {
		cout << "E: Unable to retrieve Volume handles." << endl;
	}
	
	// ********** Raid Infos ***********
	handleCount = INITIAL_COUNT;
	status = SsiGetRaidInfoHandles(session, handleList, &handleCount);
	if (status == SSI_StatusOk) {
		cout << "RaidInfos: " << handleCount << endl;
	} else {
		cout << "E: Unable to retrieve RaidInfo handles." << endl;
	}
	return 0;
}

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Handle session, session1;
	
    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        cout << "E: unable to initialize the SSI library (errcode:" << status << ")" << endl;
        return -1;
    }
    status = SsiSessionOpen(&session);
    if (status == SSI_StatusOk) {
		cout << "Session\tHandle = 0x" << hex << session << dec << endl;
		SessionStats(session);
		
		// open another session to see if handles are reused
		status = SsiSessionOpen(&session1);
		if (status == SSI_StatusOk) {
			cout << "Session1\tHandle = 0x" << hex << session1 << dec << endl;
			SessionStats(session1);
		} else {
			cout << "E: unable to open a session (errcode:" << status << ")" << endl;
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
