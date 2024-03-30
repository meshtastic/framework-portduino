#include "HardwareSPI.h"
#ifndef SPIChip_DEFINED
#define SPIChip_DEFINED

class SPIChip 
{
public:
    /**
     * Do a SPI transaction to the selected device
     * 
     * @param outBuf if NULL it will be not used (zeros will be sent)
     * @param inBuf if NULL it will not be used (device response bytes will be discarded)
     * @param deassertCS after last transaction (if not set, it will be left asserted)
     * @return 0 for success, else ERRNO fault code
     */
    virtual int transfer(const uint8_t *outBuf, uint8_t *inBuf, size_t bufLen, bool deassertCS = true) = 0;

    /// is this chip controlling real hardware?
    virtual const bool isSimulated() { return false; }
};


class SimSPIChip : public SPIChip
{
public:
    /**
     * Do a SPI transaction to the selected device
     * 
     * @param outBuf if NULL it will be not used (zeros will be sent)
     * @param inBuf if NULL it will not be used (device response bytes will be discarded)
     * @param deassertCS after last transaction (if not set, it will be left asserted)
     * @return 0 for success, else ERRNO fault code
     */
    int transfer(const uint8_t *outBuf, uint8_t *inBuf, size_t bufLen, bool deassertCS = true) {
        // log(SysSPI, LogVerbose, "SIM: spiTransfer(%d) -> %d", bufLen);
        return 0;
    }

    /// is this chip controlling real hardware?
    virtual const bool isSimulated() { return true; }    
};

#endif
