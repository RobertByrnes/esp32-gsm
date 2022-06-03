#include "DataUploadApi.h"


// Constructor (and parent constructor)
DataUploadApi::DataUploadApi(CellularNetwork800L &network, TinyGsmClientSecure &client, const string &host, const string &auth_path): 
  OAuth2(host, auth_path), modem(network), https_client(client)  {
    this->setGrantType(GRANT_TYPE, CLIENT_ID, CLIENT_SECRET);
    Serial.println("[+] Initializing modem...");
    this->modem.initSim(SIM_PIN);
  }

// Destructor
DataUploadApi::~DataUploadApi() {}


/**
 * @brief Connects via mobile network to Draper Biovillam API to collect auth token before
 * reading DHT sensor and sending all latest readings.  Resets counters.
 *
 */
void DataUploadApi::connectServer()
{
  std::string completeResponse = "";
  const char *accessToken = "";

  if (!this->makeGSMConnection()) {
    return;
  }
  
  Serial.print("[+] Connected to APN: ");
  Serial.println(APN);
  Serial.print("[+] Connecting to ");
  Serial.println(SERVER);

  if (!https_client.connect(SERVER, PORT)) {
    Serial.println("[-] Connecting to server failed");
  } else {
    Serial.println("[+] Performing HTTP POST request to OAuth Server");

    https_client.print(this->personalAccessClientTokenRequestString());

    Serial.print("[D] Request string to get token: ");
    Serial.println(this->personalAccessClientTokenRequestString());

    unsigned long timeout = millis();

    while (https_client.connected() && millis() - timeout < 10000L) {
      while (https_client.available()) {
        char response = https_client.read();
        Serial.print(response);
        completeResponse += response;
      }
    }

    https_client.stop();
    Serial.println(F("[+] Server disconnected"));

    modem.connection.gprsDisconnect();
    Serial.println(F("[+] GPRS disconnected"));

    accessToken = this->getToken(completeResponse);
    Serial.print("[+] Access token: ");
    Serial.println(accessToken);
  }
}

/**
 * @brief Connect to the mobile network.
 * 
 * @return bool
 */ 
bool DataUploadApi::makeGSMConnection()
{
  if (modem.connectNetwork()) {
      Serial.println("[+] Connected to mobile network OK");
      return true;
  } else {
      Serial.println("[-] failed to connect to mobile network");
      return false;
  }
}