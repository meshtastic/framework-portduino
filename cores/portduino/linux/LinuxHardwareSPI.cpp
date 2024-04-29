//
// Created by kevinh on 9/1/20.
//

#include "HardwareSPI.h"
#include "SPIChip.h"
#include "Utility.h"
#include "logging.h"

#include <assert.h>

#ifdef PORTDUINO_LINUX_HARDWARE

#include "linux/PosixFile.h"
#include <linux/spi/spidev.h>
#include <mutex>
#include <map>
#include <memory>



class LinuxSPIChip : public SPIChip, private PosixFile {
  private:
    std::mutex SPIMutex;
    uint32_t defaultSpeed = 2000000;
    
  public:
    LinuxSPIChip(const char *name, uint32_t default_frequency) : PosixFile(name) {
      defaultSpeed = default_frequency;
      uint8_t mode = SPI_MODE_0;
      uint8_t lsb = false;
      int status = ioctl(SPI_IOC_WR_MODE, &mode);
      assert(status >= 0);
      status = ioctl(SPI_IOC_WR_LSB_FIRST, &lsb);
      assert(status >= 0);
      status = ioctl(SPI_IOC_WR_MAX_SPEED_HZ, &defaultSpeed);
      assert(status >= 0);
    }

    /**
     * Do a SPI transaction to the selected device
     *
     * @param outBuf if NULL it will be not used (zeros will be sent)
     * @param inBuf if NULL it will not be used (device response bytes will be
     * discarded)
     * @param deassertCS after last transaction (if not set, it will be left
     * asserted)
     * @return 0 for success, else ERRNO fault code
     */
    int transfer(const uint8_t *outBuf, uint8_t *inBuf, size_t bufLen,
                bool deassertCS = true) {
      struct spi_ioc_transfer xfer;

      memset(&xfer, 0, sizeof xfer);

      xfer.tx_buf = (unsigned long)outBuf;
      xfer.len = bufLen;

      xfer.rx_buf = (unsigned long)inBuf; // Could be NULL, to ignore RX bytes
      xfer.cs_change = deassertCS;

      int status = ioctl(SPI_IOC_MESSAGE(1), &xfer);
      if (status < 0) {
        perror("SPI_IOC_MESSAGE");
        return status;
      }
      return 0;
    }
    void beginTransaction(uint32_t clockSpeed) {
      SPIMutex.lock();
      assert (ioctl(SPI_IOC_WR_MAX_SPEED_HZ, &clockSpeed) >= 0);

    }
    void endTransaction() {
      SPIMutex.unlock();
      assert (ioctl(SPI_IOC_WR_MAX_SPEED_HZ, &defaultSpeed) >= 0);
    }
};
std::map<std::string, std::shared_ptr<void>> SPI_map;
#endif

namespace arduino {

HardwareSPI::HardwareSPI(int8_t _spi_host) {
  char x = (_spi_host & (1 << 4) - 1) + '0';
  char y = (_spi_host >> 4) + '0';
  spiString = "/dev/spidev";
  spiString += x;
  spiString += ".";
  spiString += y;
}
uint8_t HardwareSPI::transfer(uint8_t data) {
  uint8_t response;
  assert(spiChip);
  spiChip->transfer(&data, &response, 1, false); // leave CS asserted
  // printf("sent 0x%x, received %0x\n", data, response);
  return response;
}

uint16_t HardwareSPI::transfer16(uint16_t data) {
  notImplemented("transfer16");
  assert(0); // make fatal for now to prevent accidental use
  return 0x4242;
}

// BIG PERFORMANCE FIXME!  Change the RadioLib transfer code to use this method
// for many fewer kernel switches/USB transactions.
// In fact - switch the API to the nrf52/esp32 arduino version that takes both
// an inbuf and an outbuf;
void HardwareSPI::transfer(void *buf, size_t count) {
  spiChip->transfer((uint8_t *) buf, (uint8_t *) buf, count, false);
}

void HardwareSPI::transfer(void *out, void *in, size_t count) {
  spiChip->transfer((uint8_t *) out, (uint8_t *) in, count, false);
}

// Transaction Functions
void HardwareSPI::usingInterrupt(int interruptNumber) {
  // Do nothing
}

void HardwareSPI::notUsingInterrupt(int interruptNumber) {
  // Do nothing
}

void HardwareSPI::beginTransaction(SPISettings settings) {
  assert(settings.bitOrder == MSBFIRST); // we don't support changing yet
  assert(settings.dataMode == SPI_MODE0);
  if (spiChip)
    spiChip->beginTransaction(settings.clockFreq);
}

void HardwareSPI::endTransaction(void) {
  if (spiChip)
    spiChip->endTransaction();
}

// SPI Configuration methods
void HardwareSPI::attachInterrupt() {
  // Do nothing
}

void HardwareSPI::detachInterrupt() {
  // Do nothing
}

void HardwareSPI::begin(uint32_t freq) {

  if (!spiChip) {
    if (SPI_map[spiString] != nullptr) {
      spiChip = std::static_pointer_cast<SPIChip>(SPI_map[spiString]);
    }

#ifdef PORTDUINO_LINUX_HARDWARE
    // FIXME, only install the following on linux and only if we see that the
    // device exists in the filesystem
    try {
      spiChip = std::make_shared<LinuxSPIChip>(spiString.c_str(), freq);
      SPI_map[spiString] = spiChip;

    } catch (...) {
      printf("No hardware spi chip found...\n");
    }
#endif

    if (!spiChip) // no hw spi found, use the simulator
      spiChip = std::make_shared<SimSPIChip>();
  }
}

void HardwareSPI::begin(const char *name, uint32_t freq) {
  if (name != nullptr)
    spiString = std::string(name);
  begin(freq);
}

void HardwareSPI::end() {
  if (spiChip) {
      spiChip.reset();

  }
  spiChip = NULL;
}

} // namespace arduino

HardwareSPI SPI;
