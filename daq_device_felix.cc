#include "daq_device_felix.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

daq_device_felix::daq_device_felix(const int eventtype,
                                   const int subeventid,
                                   const int card_nr,
                                   const int64_t buffer_size,
                                   const int dma_index,
                                   const int flag)
{
    _eventType = eventtype;
    _subEvtID = subeventid;
    _buffer_size = buffer_size;
    _maxLength = 5000000;

    // set the statuss
    _isRunning = false;
    _irqTrigger = flag & 0b001;
    _fanoutEna = flag & 0b010;
    _externalEmu = flag & 0b100;

    // Allocate the DMA buffer
    _dataBuffer = this->cmemAllocate(_buffer_size);

    // Normally this should be in a try block to catch FlxExcption
    // Since there is no clear solution yet about what to do when
    // such exception happens, thus we let it simply die here
    _flx = new FlxCard;
    _flx->card_open(card_nr);

    // Trigger logic
    if(_irqTrigger)
    {
        _th = new felixTriggerHandler(_flx);
        registerTriggerHandler(_th);
    }
    else
    {
        //should we generate a deadtime based polling trigger or use system's default one?
        _th = 0;
    }
}

daq_device_felix::~daq_device_felix()
{
    if(_isRunning) endrun();

    //close the flxcard
    _flx->card_close();

    //release the cmem buffer
    if(_dataBuffer)
    {
        munmap((void*)_cmemDescriptor.uaddr, _cmemDescriptor.size);
        ioctl(_cmemHandle, CMEM_RCC_FREE, &_cmemDescriptor.handle);
    }
}

int daq_device_felix::put_data(const int etype, int* addr, const int length)
{
    if(etype != _eventType) return 0;  //not our id
    sevt = (subevtdata_ptr)addr;

    sevt->sub_length = SEVTHEADERLENGTH;
    sevt->sub_id = _subEvtID;
    sevt->sub_decoding = IDFELIXV1;
    sevt->sub_type = 4;
    sevt->reserved[0] = 0;
    sevt->reserved[1] = 0;

    //Copy data from DMA buffer
    char* data = (char*)&(sevt->data);

    // loop or memcpy here to move data
    _currAddr = _dmaStat->current_address;
    while(_currAddr < _startAddr || _currAddr > _endAddr) _currAddr = _dmaStat->current_address;

    if(_currAddr > _prevAddr)
    {
        uint64_t len = transfer(data, _prevAddr, _currAddr);
        sevt->sub_length += len;
        data += len;
    }
    else if(_currAddr < _prevAddr)  //wrap around
    {
        uint64_t len = transfer(data, _prevAddr, _endAddr);
        sevt->sub_length += len;
        data += len;

        len = transfer(data, _startAddr, _currAddr);
        sevt->sub_length += len;
        data += len;
    } //if _currAddr == _prevAddr, do nothing
    _prevAddr = _currAddr;

    //End-of-packet flag
    const unsigned int eop_marker = 0xf000f000;
    memcpy(data, &eop_marker, 4);

    //packet padding to make packets aligned
    sevt->sub_padding = sevt->sub_length % 4;
    sevt->sub_length += sevt->sub_padding;
    return sevt->sub_length;
}

uint64_t daq_device_felix::transfer(char* dest, uint64_t headAddr, uint64_t tailAddr)
{
    uint64_t startIndex = headAddr - _startAddr;
    uint64_t len = tailAddr - headAddr;
    if(len > _maxLength) len = _maxLength;

    memcpy(dest, &_dataBuffer[startIndex], len);
    return len;
}

int daq_device_felix::init()
{
    //Start the trigger interface if needed
    if(_irqTrigger)
    {
        _flx->irq_enable(FLX_INT_DATA_AVAILABLE);
    }

    //start the data flow inside Flx
    if(_fanoutEna)
    {
        setFanOut();
    }
    else
    {
        startDataEmulator();
    }

    //start the DMA
    if(_dataBuffer) this->dmaStart(_cmemPhysAddr, _buffer_size, _dma_index);

    _isRunning = true;
    _currAddr = _startAddr;
    _prevAddr = _startAddr;
    return 0;
}

int daq_device_felix::endrun()
{
    if(_irqTrigger )
    {
        _flx->irq_disable(FLX_INT_DATA_AVAILABLE);
        _flx->irq_cancel(FLX_INT_DATA_AVAILABLE);
    }

    if(!_fanoutEna) stopDataEmulator();
    this->dmaStop(_dma_index);

    _isRunning = false;
    return 0;
}

int daq_device_felix::rearm(const int etype)
{
    return 0;
}

void daq_device_felix::identify(std::ostream& os) const
{
    os << "FELIX Device  Event Type: " << _eventType << " Subevent id: " << _subEvtID
       << " Firmware Ver: " << firmwareVersion();
    if(_irqTrigger) os << " trigger ";
    os << std::endl;
}

int daq_device_felix::max_length(const int etype) const
{
    if(etype != _eventType) return 0;
    return _maxLength;
}

//------------------- Below are private functions directly taken from fdaq code ------------------
void daq_device_felix::setFanOut()
{
    this->stopDataEmulator();

    bool locked = this->fanOutLocked();
    if(!locked)
    {
        _flx->cfg_set_option(BF_GBT_UPLNK_FO_SEL, 0);
        _flx->cfg_set_option(BF_GBT_DNLNK_FO_SEL, 0);
    }
}

void daq_device_felix::startDataEmulator()
{
    if(_fanoutEna) return;
    if(_flx == 0) return;

    bool locked = this->fanOutLocked();
    if(_externalEmu)
    {
        if(!locked)
        {
            // Select real GBT link for all CentralRouter channels
            _flx->cfg_set_option(BF_GBT_DNLNK_FO_SEL, 0);

            // Enable GBT_EMU for all GBTs ('to frontend')
            _flx->cfg_set_option( BF_GBT_UPLNK_FO_SEL, 0xFFFFFF);
        }

        // 'TOFRONTEND' emulator enable
        _flx->cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 0);
        _flx->cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 1);
        _flx->cfg_set_option(BF_GBT_FM_EMU_ENA_TOHOST, 0); // FULL-mode
    }
    else
    {
        if(!locked)
        {
            // Enable GBT_EMU for all GBTs ('to host')
            _flx->cfg_set_option(BF_GBT_DNLNK_FO_SEL, 0xFFFFFF);

            // Select CentralRouter data for all GBT links
            _flx->cfg_set_option(BF_GBT_UPLNK_FO_SEL, 0);
        }

        if(_flx->cfg_get_reg(REG_FIRMWARE_MODE) == 1)
        {
            // 'TOHOST' FULL-mode emulator enable
            _flx->cfg_set_option(BF_GBT_FM_EMU_ENA_TOHOST, 1);
        }
        else
        {
            // 'TOHOST' emulator enable
            _flx->cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 0);
            _flx->cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 1);
        }
    }
}

void daq_device_felix::stopDataEmulator()
{
    // Stop the data emulator on the FLX-card
    if(_flx == 0) return;

    // Disable emulators
    _flx->cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 0);
    _flx->cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 0);
    _flx->cfg_set_option(BF_GBT_FM_EMU_ENA_TOHOST, 0); // FULL-mode

    bool locked = this->fanOutLocked();
    if(!locked)
    {
        // Select CentralRouter data for all GBT links
        // ("up/downlink fanout select")
        _flx->cfg_set_option(BF_GBT_UPLNK_FO_SEL, 0);
        // Slowly crap data enters the CR! (when GTH-links not connected?),
        // so outcommented:
        //_flx->cfg_set_option( BF_GBT_DNLNK_FO_SEL, 0 );
        _flx->cfg_set_option(BF_GBT_DNLNK_FO_SEL, 0xFFFFFF);
    }
}

bool daq_device_felix::fanOutLocked()
{
    bool locked = false;
    if(_flx->cfg_get_reg( REG_GBT_DNLNK_FO_SEL ) & 0x80000000) locked = true;
    if(_flx->cfg_get_reg( REG_GBT_UPLNK_FO_SEL ) & 0x80000000) locked = true;
    return locked;
}

char* daq_device_felix::cmemAllocate(uint64_t buffer_size)
{
    char* buffer = 0;

    // Open CMEM(_RCC) and allocate the requested buffer size
    if((_cmemHandle = open("/dev/cmem_rcc", O_RDWR)) < 0)
    {
        return 0;
    }

    sprintf(_cmemDescriptor.name, "FlxDaq");
    _cmemDescriptor.size  = _buffer_size;
    _cmemDescriptor.order = 0;
    _cmemDescriptor.type  = TYPE_GFPBPA;
    if(CMEM_RCC_SUCCESS == ioctl(_cmemHandle, CMEM_RCC_GET, &_cmemDescriptor))
    {
        int64_t result = (int64_t)mmap(0, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, _cmemHandle, (int64_t)_cmemDescriptor.paddr);
        if(result != -1)
        {
            _cmemDescriptor.uaddr = result;
            if(CMEM_RCC_SUCCESS == ioctl(_cmemHandle, CMEM_RCC_SETUADDR, &_cmemDescriptor)) buffer = (char *)_cmemDescriptor.uaddr;

            _cmemPhysAddr = _cmemDescriptor.paddr;
            _cmemVirtAddr = _cmemDescriptor.uaddr;
        }

        if(buffer == 0) return 0;
    }
    else
    {
        return 0;
    }

    _startAddr = _cmemPhysAddr;
    _endAddr   = _startAddr + buffer_size;

    return buffer;
}

void daq_device_felix::dmaStart(uint64_t dst_phys_addr, uint64_t size, int dma_index)
{
    if(!(_flx != 0 && dst_phys_addr != 0 && size > 0)) return;

    // Just in case...
    this->stopDataEmulator();
    this->dmaStop(dma_index);

    uint64_t bar0_addr = _flx->openBackDoor(0);
    volatile flxcard_bar0_regs_t* bar0 = (volatile flxcard_bar0_regs_t*)bar0_addr;
    _dmaDesc = &bar0->DMA_DESC[dma_index];
    _dmaStat = &bar0->DMA_DESC_STATUS[dma_index];

    // Enable data-available interrupt
    _flx->irq_enable(FLX_INT_DATA_AVAILABLE);

    int tlp = _flx->dma_max_tlp_bytes();
    _tlpSizeMask = ~((uint64_t)tlp - 1);

    _flx->dma_to_host(dma_index, dst_phys_addr, size, FLX_DMA_WRAPAROUND);
    usleep(1000); // If we don't, _readPtr is 0 after the next statement!?
}

void daq_device_felix::dmaStop(int dma_index)
{
    if(_flx == 0) return;
    _flx->dma_stop(dma_index);

    // Reset and flush
    _flx->dma_reset();
    _flx->dma_fifo_flush();
    _flx->soft_reset();
}

std::string daq_device_felix::firmwareVersion() const
{
    if(_flx == 0) return std::string();

    std::ostringstream oss;
    uint64_t ts  = _flx->cfg_get_reg(REG_BOARD_ID_TIMESTAMP);
    uint64_t svn = _flx->cfg_get_reg(REG_BOARD_ID_SVN);
    uint64_t fm  = _flx->cfg_get_reg(REG_FIRMWARE_MODE);
    uint64_t chn = _flx->cfg_get_reg(REG_NUM_OF_CHANNELS);
    uint64_t typ = _flx->cfg_get_reg(REG_CARD_TYPE);

    oss << std::hex << ts << "-" << std::dec << svn;
    if(fm == 1)
    {
        oss << "-FM-";
    }
    else
    {
        oss << "-GBT-";
    }
    oss << chn << "ch-" << typ;

    return oss.str();
}
