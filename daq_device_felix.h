#ifndef __DAQ_DEVICES_FELIX_H_
#define __DAQ_DEVICES_FELIX_H_

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <boost/lexical_cast.hpp>

#include <daq_device.h>
#include "felixTriggerHandler.h"

#include <cmem_rcc/cmem_rcc.h>
#include <flxcard/FlxCard.h>
#include "flxdefs.h"

class daq_device_felix : public daq_device
{
public:
    daq_device_felix(const int eventtype,
                     const int subeventid,
                     const int card_nr,
                     const int64_t buffer_size,
                     const int dma_index,
                     const int flag = 0);  //bit 0 for trigger enable, bit 1 for fanout enable, bit 2 for ext emu enable
    ~daq_device_felix();

    //! answer to daq_status request
    void identify(std::ostream& os = std::cout) const;

    //! Get maximum event length
    int max_length(const int etype) const;

    //! core function to fetch data and pack with envelop info
    int put_data(const int etype, int* addr, const int length);

    //!start/stop and restart the device
    //@{
    int init();   //! called at the start of run
    int endrun(); //! called at the end of run
    int rearm(const int etype); //! don't know what it does
    //@}

private:
    //! Allocate the DMA memory buffer
    char* cmemAllocate(uint64_t buffer_size);

    //! start and stop the DMA
    void dmaStart(uint64_t dst_phys_addr, uint64_t size, int dma_index = 0);
    void dmaStop(int dma_index = 0);

    //! check the FO_SEL lock bit
    bool fanOutLocked();

    //! start/stop the data emulator -- probably needs clean up after R&D stage
    void startDataEmulator();
    void stopDataEmulator();

    //! set fan out
    void setFanOut();

    //! get the firmware version
    std::string firmwareVersion();

protected:
    subevtdata_ptr sevt;
    unsigned int number_of_words;

    //! event type specification
    int _eventType;
    int _subEvtID;

    //! points to the trigger handler
    felixTriggerHandler* _th;

    //! running state indicator
    //@{
    bool _isRunning;
    bool _irqTrigger;
    bool _fanoutEna;
    bool _externalEmu;
    //@}

    //! actual data reader
    FlxCard* _flx;

    //! pointer to the allocated receive buffer
    char* _dataBuffer;

    //!CMEM and DMA stuff  -- see FlxReceiver
    //@{
    uint64_t   _buffer_size;
    uint64_t   _tlpSizeMask;
    int        _dma_index;

    cmem_rcc_t _cmemDescriptor;
    uint64_t   _cmemPhysAddr;
    uint64_t   _cmemVirtAddr;
    int        _cmemHandle;

    int                        _dmaIndex;
    volatile dma_descriptor_t* _dmaDesc;
    volatile dma_status_t*     _dmaStat;

    uint64_t _startAddr, _endAddr;
    uint64_t _currAddr, _prevAddr;
    //@}
};

#endif
