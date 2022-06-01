#include <Arduino.h>
#include "config.h"
#include <CellularNetwork800L.h>
#include <OAuth2.h>
#include <StreamDebugger.h>
#include "WiFi_FirmwareUpdater.h"


// Modem
TinyGsm modem(SerialAT);
CellularNetwork800L network(APN, GPRS_USER, GPRS_PASSWORD, modem); // explore passing modem by reference

// HTTP(S) Client
TinyGsmClientSecure client(modem);
// HttpClient http(client, HOST, 443);

// Authentication
OAuth2 auth(OAUTH_HOST, OAUTH_TOKEN_PATH);

//  Updates
WiFi_FirmwareUpdater update(SSID, PASSWORD);

/**
 * @brief Connect to the mobile network.
 * 
 * @return bool
 */ 
bool makeGSMConnection()
{
  if (network.connectNetwork()) {
      Serial.println("[+] Connected to mobile network OK");
      return true;
  } else {
      Serial.println("[-] failed to connect to mobile network");
      return false;
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);

  // Serial.println("[+] Initializing modem...");
  // network.initSim(SIM_PIN);

  // auth.setGrantType(GRANT_TYPE, CLIENT_ID, CLIENT_SECRET);
}

/**
 * @brief Connects via mobile network to Draper Biovillam API to collect auth token before
 * reading DHT sensor and sending all latest readings.  Resets counters.
 *
 */
void connectServer()
{
  std::string completeResponse = "";
  const char *accessToken = "";

  makeGSMConnection();
  // if (network.isNetworkConnected()) {
  //   Serial.print("[+] Connected to APN: ");
  // }
  
  Serial.println(APN);
  Serial.print("[+] Connecting to ");
  Serial.println(SERVER);

  if (!client.connect(SERVER, PORT)) {
    Serial.println("[-] Connecting to server failed");
  } else {
    Serial.println("[+] Performing HTTP POST request to OAuth Server");

    client.print(auth.personalAccessClientTokenRequestString());

    Serial.print("[D] Request string to get token: ");
    Serial.println(auth.personalAccessClientTokenRequestString());

    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 10000L) {
      while (client.available()) {
        char response = client.read();
        Serial.print(response);
        completeResponse += response;
      }
    }

    client.stop();
    Serial.println(F("[+] Server disconnected"));

    network.connection.gprsDisconnect();
    Serial.println(F("[+] GPRS disconnected"));

    accessToken = auth.getToken(completeResponse);
    Serial.print("[+] Access token: ");
    Serial.println(accessToken);
  }
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
