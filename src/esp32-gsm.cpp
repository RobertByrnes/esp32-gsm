#include <Arduino.h>
#include "config.h"
#include <CellularNetwork800L.h>
#include <DataUploadApi.h>
#include <StreamDebugger.h>
#include "WiFi_FirmwareUpdater.h"


// Modem
TinyGsm modem(SerialAT);
CellularNetwork800L network(APN, GPRS_USER, GPRS_PASSWORD, modem); // explore passing modem by reference

// HTTP(S) Client
TinyGsmClientSecure client(modem);

// Authentication
DataUploadApi api(network, client, OAUTH_HOST, OAUTH_TOKEN_PATH);

//  Updates
WiFi_FirmwareUpdater update(SSID, PASSWORD);



void setup()
{
  Serial.begin(BAUD_RATE);

  // Serial.println("[+] Initializing modem...");
  // network.initSim(SIM_PIN);

  // auth.setGrantType(GRANT_TYPE, CLIENT_ID, CLIENT_SECRET);
}

void loop()
{
  if (millis() - PREVIOUS_MILLIS >= CONNECTION_INTERVAL) {
    PREVIOUS_MILLIS = millis();
    RESTART_COUNTER++;
    // connectServer();
    // makeGSMConnection();

    if (update.checkUpdateAvailable(UPDATE_VERSION_FILE_URL)) {
      update.updateFirmware(UPDATE_URL);
    }
    
    
    sleep(60);
    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }
}
