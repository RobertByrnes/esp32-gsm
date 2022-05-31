#ifndef GSM_FIRMWARE_UPDATER_H
#define GSM_FIRMWARE_UPDATER_H

#include "CellularNetwork.h"
#include "OAuth2.h"
#include <CRC32.h>
#include <Update.h>

class GSMFirmwareUpdater
{
public:
    int totalLength;
    int currentLength;

    GSMFirmwareUpdater();
    ~GSMFirmwareUpdater();

    void performUpdate(const char *UPDATE_URL, const char *UPDATE_HOST, const uint16_t PORT, CRC32 &crc, TinyGsmClientSecure &client, CellularNetwork &network);
private:
    void printPercent(uint32_t readLength, uint32_t contentLength);
    void updateFirmware(uint8_t *data, size_t len);
};


#endif