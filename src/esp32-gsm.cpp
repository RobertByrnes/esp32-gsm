#include <Arduino.h>
#include "config.h"
#include <DataUploadApi.h>
#include "WiFi_FirmwareUpdater.h"


// Modem
TinyGsm modem(SerialAT);
CellularNetwork800L network(APN, GPRS_USER, GPRS_PASSWORD, modem);

// HTTP(S) Client
TinyGsmClientSecure client(modem);

// Authentication
DataUploadApi api(network, client, OAUTH_HOST, OAUTH_TOKEN_PATH);

//  Updates
WiFi_FirmwareUpdater update(SSID, PASSWORD);



void setup()
{
  Serial.begin(BAUD_RATE);
}

void loop()
{
  if (millis() - PREVIOUS_MILLIS >= CONNECTION_INTERVAL) {
    PREVIOUS_MILLIS = millis();
    RESTART_COUNTER++;

    api.connectServer();
    
    sleep(60);
    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }

  if (millis() - PREVIOUS_MILLIS >= 100000U) {
    PREVIOUS_MILLIS = millis();
    RESTART_COUNTER++;

    if (update.checkUpdateAvailable(UPDATE_VERSION_FILE_URL)) {
      update.updateFirmware(UPDATE_URL);
    }
    
    sleep(60);
    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }
}
