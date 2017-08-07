#ifndef __FELIX_PLUGIN_H__
#define __FELIX_PLUGIN_H__

#include <iostream>

#include <rcdaq_plugin.h>
#include "daq_device_felix.h"

class felix_plugin : public RCDAQPlugin
{
public:
    int create_device(deviceblock* db);
    void identify(std::ostream& os = std::cout, const int flag = 0) const;
};

#endif
