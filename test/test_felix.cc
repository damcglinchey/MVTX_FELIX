#include <iostream>
#include "daq_device_felix.h"

#define logInfo(message) cout << __LINE__ << " : " << message << endl;

using namespace std;

int main(int argc, char* argv[])
{
    daq_device_felix* felix = new daq_device_felix(1, 1000, 0, 1024, 0, 4);
    felix->identify();

    logInfo("start of run");
    felix->init();

    logInfo("take one event");
    int* data = new int[100000];
    felix->put_data(1, data, 100000);

    logInfo("end of run");
    felix->endrun();

    return 0;
}
