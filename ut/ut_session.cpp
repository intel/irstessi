// ex: tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab:

#include <iostream>
#include <ssi.h>

using namespace std;

int main(int argc, char *argv[])
{
    SSI_Status status;
    SSI_Handle handle;

    status = SsiInitialize();
    if (status != SSI_StatusOk) {
        cout << "E: unable to initialize the SSI library (errcode:" << status << ")" << endl;
        return -1;
    }
    status = SsiSessionOpen(&handle);
    if (status == SSI_StatusOk) {
        status = SsiSessionClose(handle);
        if (status != SSI_StatusOk) {
            cout << "E: unable to close the session=0x" << hex << handle << " (errcode:" << status << ")" << endl;
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
