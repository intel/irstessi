// ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

#include <iostream>

#include "exception.h"
#include "list.h"
#include "string.h"
#include "filesystem.h"

using namespace std;

int main(int argc, char *argv[])
{
    SysfsAttr attr("/sys/bus/pci/devices/0000:00:1f.2/uevent");
    String content;

    attr >> content;
    cout << content.get() << endl;

    return 0;
}
