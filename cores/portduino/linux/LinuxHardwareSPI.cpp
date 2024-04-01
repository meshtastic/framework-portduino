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

class LinuxSPIChip : public SPIChip, private PosixFile {
public:
  LinuxSPIChip(const char *name) : PosixFile(name) {
    uint8_t mode = SPI_MODE_0;
    uint8_t lsb = false;
    uint32_t speed = 2000000;
    int status = ioctl(SPI_IOC_WR_MODE, &mode);
    assert(status >= 0);
    status = ioctl(SPI_IOC_WR_LSB_FIRST, &lsb);
    assert(status >= 0);
    status = ioctl(SPI_IOC_WR_MAX_SPEED_HZ, &speed);
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

    /*
    We want default SPI mode 0, MSB first

    uint8_t mode = SPI_MODE_0;
    uint8_t lsb = 0; // we want MSB first SPI_LSB_FIRST;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb);
    */

    memset(&xfer, 0, sizeof xfer);

    xfer.tx_buf = (unsigned long)outBuf;
    xfer.len = bufLen;

    xfer.rx_buf = (unsigned long)inBuf; // Could be NULL, to ignore RX bytes
    xfer.cs_change = deassertCS;
    xfer.delay_usecs = 100;

    int status = ioctl(SPI_IOC_MESSAGE(1), &xfer);
    if (status < 0) {
      perror("SPI_IOC_MESSAGE");
      return status;
    }

    /* printf("SPI response(%d): ", status);
    size_t len = bufLen;
    for (auto bp = inBuf; len; len--)
        printf("%02x ", *bp++);
    printf("\n"); */

    return 0;
  }
};
#endif

namespace arduino {

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
  spiChip->transfer((uint8_t *) buf, (uint8_t *) buf, count);
}

void HardwareSPI::transfer(void *out, void *in, size_t count) {
  spiChip->transfer((uint8_t *) out, (uint8_t *) in, count);
}

// Transaction Functions
void HardwareSPI::usingInterrupt(int interruptNumber) {
  // Do nothing
}

void HardwareSPI::notUsingInterrupt(int interruptNumber) {
  // Do nothing
}

void HardwareSPI::beginTransaction(SPISettings settings) {
  // Do nothing
  // printf("beginTransaction\n");
  spiChip->transfer(NULL, NULL, 0, false); // turn on chip select
  assert(settings.bitOrder == MSBFIRST); // we don't support changing yet
  assert(settings.dataMode == SPI_MODE0);
}

void HardwareSPI::endTransaction(void) {
  assert(spiChip);

  // FIXME - for the time being I'm not using automatic ship select management
  spiChip->transfer(NULL, NULL, 0, true); // turn off chip select

  // printf("endTransaction\n");
}

// SPI Configuration methods
void HardwareSPI::attachInterrupt() {
  // Do nothing
}

void HardwareSPI::detachInterrupt() {
  // Do nothing
}

void HardwareSPI::begin(const char *name) {
  // We only do this init once per boot
  if (!spiChip) {

#ifdef PORTDUINO_LINUX_HARDWARE
    // FIXME, only install the following on linux and only if we see that the
    // device exists in the filesystem
    try {
      spiChip = new LinuxSPIChip(name);
    } catch (...) {
      printf("No hardware spi chip found...\n");
    }
#endif

    if (!spiChip) // no hw spi found, use the simulator
      spiChip = new SimSPIChip();
  }
}

void HardwareSPI::end() {
  if (spiChip) {
#ifdef PORTDUINO_LINUX_HARDWARE
    if (!spiChip->isSimulated())
      delete (LinuxSPIChip*)spiChip;
    else
      delete spiChip;
#else
      delete spiChip;
#endif
  }
  spiChip = NULL;
}

} // namespace arduino

HardwareSPI SPI;
