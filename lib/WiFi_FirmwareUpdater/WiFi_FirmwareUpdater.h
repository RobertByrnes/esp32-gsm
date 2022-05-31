#ifndef WIFI_FIRMWARE_UPDATER_H
#define WIFI_FIRMWARE_UPDATER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

class WiFi_FirmwareUpdater: public HTTPClient
{
public:
    const char* ssid; // Local network name.
    const char* password; // Local network password.

    WiFi_FirmwareUpdater(const char* ssid, const char* password);
    ~WiFi_FirmwareUpdater();

    void updateFirmware(const char *updateUrl);

private:
    int totalLength; // Total size of firmware.
    int currentLength; // Current size of the written firmware.

    void connectWifi();
    void processUpdate(uint8_t *data, size_t len);
};

#endif
