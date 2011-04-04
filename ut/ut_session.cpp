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
	unsigned int handleCount;

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
			cout << "Controllers: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve Controller handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetEndDeviceHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "EndDevices: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve EndDevice handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetPhyHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "Phys: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve Phy handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetPortHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "Ports: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve Port handles." << endl;
		}
		handleCount = INITIAL_COUNT;
		status = SsiGetRoutingDeviceHandles(handle, SSI_ScopeTypeNone, SSI_NULL_HANDLE, handleList, &handleCount);
		if (status == SSI_StatusOk) {
			cout << "RoutingDevices: " << handleCount << endl;
		} else {
			cout << "E: Unable to retrieve RoutingDevice handles." << endl;
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