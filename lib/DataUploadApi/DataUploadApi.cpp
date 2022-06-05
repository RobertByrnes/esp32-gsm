#include "DataUploadApi.h"


// Serial output configuration - uncomment if debugging through the serial port
#define SERIAL_CONNECTED

// Default server timeout
#define SERVER_TIMEOUT 60000U

// SIM card PIN, leave empty if not defined
const char *SIM_PIN = "";

// Constructor (and parent constructor)
DataUploadApi::DataUploadApi(CellularNetwork800L &network, TinyGsmClientSecure &client, const string &host, const string &auth_path): 
  OAuth2(host, auth_path), modem(network), https_client(client)  {
    this->setGrantType(GRANT_TYPE, CLIENT_ID, CLIENT_SECRET);
#ifdef SERIAL_CONNECTED
    Serial.println("[+] Initializing modem...");
#endif
    this->modem.initSim(SIM_PIN);
  }

// Destructor
DataUploadApi::~DataUploadApi() {}


/**
 * @brief Connects via mobile network to Draper Biovillam API to collect auth token before
 * reading DHT sensor and sending all latest readings.  Resets counters.
 *
 */
void DataUploadApi::connectServer(const char *apn, const char *server, const uint16_t &port)
{
  std::string completeResponse = "";
  std::string accessToken = "";

  if (!this->makeGSMConnection()) {
    return;
  }

#ifdef SERIAL_CONNECTED
  Serial.print("[+] Connected to APN: ");
  Serial.println(apn);
  Serial.print("[+] Connecting to ");
  Serial.println(server);
#endif

  if (!https_client.connect(server, port)) {
#ifdef SERIAL_CONNECTED
    Serial.println("[-] Connecting to server failed");
#endif
  } else {
#ifdef SERIAL_CONNECTED
    Serial.println("[+] Performing HTTP POST request to OAuth Server");
#endif
    https_client.print(this->personalAccessClientTokenRequestString());
#ifdef SERIAL_CONNECTED
    Serial.print("[D] Request string to get token: ");
    Serial.println(this->personalAccessClientTokenRequestString());
#endif
    unsigned long timeout = millis();

    while (https_client.connected() && millis() - timeout < SERVER_TIMEOUT) {
      while (https_client.available()) {
        char response = https_client.read();
        Serial.print(response);
        completeResponse += response;
      }
    }

    https_client.stop();
#ifdef SERIAL_CONNECTED
    Serial.println(F("[+] Server disconnected"));
#endif
    modem.connection.gprsDisconnect();
#ifdef SERIAL_CONNECTED
    Serial.println(F("[+] GPRS disconnected"));
#endif
    accessToken = this->getToken(completeResponse);
#ifdef SERIAL_CONNECTED
    Serial.print("[D] Response: ");
    Serial.println(completeResponse.c_str());
    Serial.print("[+] Access token: ");
    Serial.println(String(accessToken.c_str()));
#endif
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
#ifdef SERIAL_CONNECTED
    Serial.println("[+] Connected to mobile network OK");
#endif
    return true;
  } else {
#ifdef SERIAL_CONNECTED
    Serial.println("[-] failed to connect to mobile network");
#endif
    return false;
  }
}