#ifndef __DAQ_DEVICES_FELIX_H_
#define __DAQ_DEVICES_FELIX_H_

#include <iostream>

#include <daq_device.h>
#include <felixTriggerHandler.h>

class daq_device_felix : public daq_device
{
public:
    daq_device_felix(const int eventtype,
                     const int subeventid,
                     const int trigger = 0);
    ~daq_device_felix();

    //! answer to daq_status request
    void identify(std::ostream& os = std::cout) const;

    //! Get maximum event length
    int max_length(const etype) const;

    //! core function to fetch data and pack with envelop info
    int put_data(const int etype, int* addr, const int length);

    //!@name start/stop and restart the device
    //@{
    int init();
    int endrun();
    int rearm(const int etype);
    //@}

protected:
    subevtdata_ptr sevt;
    unsigned int number_of_words;

    felixTriggerHandler* _th;
};

#endif
