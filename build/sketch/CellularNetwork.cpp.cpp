#include <Arduino.h>
#line 1 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
#line 1 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
#include "CellularNetwork.h"
#include <TinyGsmClient.h>

// Constructor
CellularNetwork::CellularNetwork(const char *apn, const char *gprs_user, const char *gprs_password, TinyGsm modem):
  apn(apn), gprs_user(gprs_user), gprs_password(gprs_password), connection(modem) {}

// Destructor
CellularNetwork::~CellularNetwork() {}

bool CellularNetwork::initSim(const char simPin) {
   // Set GSM module baud rate and UART pins
  SerialAT.begin(BAUD_RATE, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  this->connection.restart();

  // Unlock SIM card with a PIN if needed
  if (strlen(simPin) && this->connection.getSimStatus() != 3) {
    this->connection.simUnlock(simPin);
#line 21 "c:\\code\\toit-sim800l-1\\main.ino"
void setup();
#line 77 "c:\\code\\toit-sim800l-1\\main.ino"
void connectServer();
#line 137 "c:\\code\\toit-sim800l-1\\main.ino"
void beginFirwareUpdate();
#line 186 "c:\\code\\toit-sim800l-1\\main.ino"
void loop();
#line 21 "c:\\code\\toit-sim800l-1\\main.ino"
  }

  return true;
}

/**
 * @brief Make up to 5 attempts to connect to the mobile network.
 * 
 * @return bool
 */
bool CellularNetwork::connectNetwork()
{
  int i = 0;
  while (i <= 5) {
    ++i;
    if (!this->connection.gprsConnect(this->apn, this->gprs_user, this->gprs_password)) {
      delay(3000);
    } else {
      return true;
    }
  }
  return false;
}
#line 1 "c:\\code\\toit-sim800l-1\\main.ino"
#include <Arduino.h>
#include "config.h"
#include "CellularNetwork.h"
#include "OAuth2.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <StreamDebugger.h>


// INSTANTIATIONS
TinyGsm modem(SerialAT);

CellularNetwork network(APN, GPRS_USER, GPRS_PASSWORD, SIM_PIN, modem); // explore passing modem by reference

TinyGsmClientSecure client(modem);

OAuth2 authHandler(OAUTH_HOST, OAUTH_TOKEN_PATH);


void setup()
{
  if (DEV) Serial.begin(BAUD_RATE);

  // Set modem enable, reset and power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  if (DEV) Serial.println("[+] Initializing modem...");
  
  network.initSim(SIM_PIN);



  // Serial.begin(115200);


  // // Start WiFi connection
  // WiFi.mode(WIFI_MODE_STA);
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(".");
  // }


}

// // Function to update firmware incrementally
// // Buffer is declared to be 128 so chunks of 128 bytes
// // from firmware is written to device until server closes
// void updateFirmware(uint8_t *data, size_t len)
// {
//   Update.write(data, len);
//   currentLength += len;
//   // Print dots while waiting for update to finish
//   Serial.print('.');
//   // if current length of written firmware is not equal to total firmware size, repeat
//   if (currentLength != totalLength)
//     return;
//   Update.end(true);
//   Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
//   // Restart ESP32 to see changes
//   ESP.restart();
// }

/**
 * @brief Connects via mobile network to Draper Biovillam API to collect auth token before
 * reading DHT sensor and sending all latest readings.  Resets counters.
 *
 */
void connectServer()
{
  String completeResponse = "";
  String accessToken = "";

  Serial.printf("[+] Connecting to APN: ", APN);

  if (network.connectNetwork()) {
      Serial.println("[+] Connection to mobile network OK");
  } else {
      Serial.println("[-] failed to connect to mobile network");
  }

  if (DEV) Serial.printf("[+] Connecting to ", SERVER);

  if (!client.connect(SERVER, PORT)) {
    if (DEV) Serial.println("[-] Connecting to server failed");
  } else {
    if (DEV) Serial.println("[+] Performing HTTP POST request to OAuth Server");

    client.print(authHandler.personalAccessClientTokenRequestString().c_str());

    if (DEV) Serial.printf("[D] Request string to get token: ", authHandler.personalAccessClientTokenRequestString(), "\n");

    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 10000L) {

      while (client.available()) {

        char response = client.read();

        if (DEV) {
          Serial.print(response);
        } else {
          Serial.println("[-] Error in response");
        }
        completeResponse.concat(response);
      }
    }

    client.stop();
    modem.gprsDisconnect();

    if (DEV) {
      Serial.println(F("[+] Server disconnected"));
      Serial.println(F("[+] GPRS disconnected"));
    }

    String parsedResponse = authHandler.findJson(completeResponse);

    if (parsedResponse != "") {
      accessToken = authHandler.extractToken(parsedResponse);
      if (DEV) Serial.println("[+] Access token: " + accessToken);
    }

    timeout = millis();
  }
}

void beginFirwareUpdate() {
  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());

  // // Connect to external web server
  // client.connect(HOST, 80);
  // // Get file, just to check if each reachable
  // int resp = client.GET();
  // Serial.print("Response: ");
  // Serial.println(resp);
  // // If file is reachable, start downloading
  // if (resp == 200) {
  //   // get length of document (is -1 when Server sends no Content-Length header)
  //   totalLength = client
  //   totalLength = client.getSize();
  //   // transfer to local variable
  //   int len = totalLength;
  //   // this is required to start firmware update process
  //   Update.begin(UPDATE_SIZE_UNKNOWN);
  //   Serial.printf("FW Size: %u\n", totalLength);
  //   // create buffer for read
  //   uint8_t buff[128] = {0};
  //   // get tcp stream
  //   WiFiClient *stream = client.getStreamPtr();
  //   // read all data from server
  //   Serial.println("Updating firmware...");
  //   while (client.connected() && (len > 0 || len == -1)) {
  //     // get available data size
  //     size_t size = stream->available();
  //     if (size) {
  //       // read up to 128 byte
  //       int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
  //       // pass to function
  //       updateFirmware(buff, c);
  //       if (len > 0) {
  //         len -= c;
  //       }
  //     }
  //     delay(1);
  //   }
  // } else {
  //   Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
  // }
  // client.stop();
  // }
}

void loop()
{
  if (millis() - PREVIOUS_MILLIS >= CONNECTION_INTERVAL) {
    PREVIOUS_MILLIS = millis();
    RESTART_COUNTER++;
    connectServer();
    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }
}
