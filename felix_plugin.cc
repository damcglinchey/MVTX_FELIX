#include "felix_plugin.h"

#include <stdint.h>
#include <boost/lexical_cast.hpp>


int felix_plugin::create_device(deviceblock* db)
{
    if(strcasecmp(db->argv0, "device_felix") != 0) return -1;
    if(db->npar != 7) return -1;  //all 6 parameters need to be explicitly set

    int eventtype = boost::lexical_cast<int>(db->argv1);
    int subid     = boost::lexical_cast<int>(db->argv2);
    int card_nr   = boost::lexical_cast<int>(db->argv3);
    int64_t buffer_size = boost::lexical_cast<int64_t>(db->argv4);
    int dma_index = boost::lexical_cast<int>(db->argv5);
    int deadtime  = boost::lexical_cast<int>(db->argv6);

    //Convert buffersize from MB to B
    buffer_size = buffer_size*1024*1024;

    daq_device* x = new daq_device_felix(eventtype, subid, card_nr, buffer_size, dma_index, deadtime);
    add_readoutdevice(x);

    return 0;
}

void felix_plugin::identify(std::ostream& os, const int flag) const
{
    if(flag <= 2)
    {
        os << " - FELIX Plugin" << std::endl;
    }
    else
    {
        os << " - FELIX Plugin, provides - " << std::endl;
        os << " -       device_felix(evttype, subid, card_nr, buffer_size, dma_index) - readout a FELIX card" << std::endl;
    }
}

felix_plugin _flxplgn;   //This is needed so that dlopen() will automatically call felix_plugin::felix_plugin() to register the device
