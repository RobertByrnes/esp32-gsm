#include <Arduino.h>
#include "config.h"
#include <CellularNetwork.h>
#include <OAuth2.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <Update.h>
#include <StreamDebugger.h>
#include "GSMFirmwareUpdater.h"


// Modem
TinyGsm modem(SerialAT);
CellularNetwork network(APN, GPRS_USER, GPRS_PASSWORD, modem); // explore passing modem by reference

// HTTP(S) Client
TinyGsmClientSecure client(modem);
HttpClient http(client, HOST, 443);

// Authentication
OAuth2 authHandler(OAUTH_HOST, OAUTH_TOKEN_PATH);

//  Updates
GSMFirmwareUpdater update;

/**
 * @brief Connect to the mobile network.
 * 
 * @return void
 */ 
void makeGSMConnection()
{
  if (network.connectNetwork()) {
      Serial.println("[+] Connected to mobile network OK");
  } else {
      Serial.println("[-] failed to connect to mobile network");
  }
}

/**
 * @brief Set modem enable, reset and power pins.
 * 
 * @return void
 */ 
void setupSIM800L_GPIO()
{
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
}

/**
 * @brief Start WiFi connection.
 * 
 * @return void
 */ 
// void setupWifiConnection()
// {
//   WiFi.mode(WIFI_MODE_STA);
//   WiFi.begin(SSID, PASSWORD);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//   }
// }

void setup()
{
  Serial.begin(BAUD_RATE);

  setupSIM800L_GPIO();

  Serial.println("[+] Initializing modem...");

  network.initSim(SIM_PIN);

  authHandler.setGrantType(GRANT_TYPE, CLIENT_ID, CLIENT_SECRET);
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

  Serial.print("[+] Connected to APN: ");
  Serial.println(APN);
  Serial.print("[+] Connecting to ");
  Serial.println(SERVER);

  if (!client.connect(SERVER, PORT)) {
    Serial.println("[-] Connecting to server failed");
  } else {
    Serial.println("[+] Performing HTTP POST request to OAuth Server");

    client.print(authHandler.personalAccessClientTokenRequestString());

    Serial.print("[D] Request string to get token: ");
    Serial.println(authHandler.personalAccessClientTokenRequestString());

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

    accessToken = authHandler.getToken(completeResponse);
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

    makeGSMConnection();

    if (client.connect(HOST, PORT)) {
      update.performUpdate(UPDATE_URL, http, client);
    }

    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }
}
