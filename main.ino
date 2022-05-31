#include <Arduino.h>
#include "config.h"
#include "CellularNetwork.h"
#include "OAuth2.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoHttpClient.h>
#include <Update.h>
#include <StreamDebugger.h>


// Modem
TinyGsm modem(SerialAT);
CellularNetwork network(APN, GPRS_USER, GPRS_PASSWORD,  modem); // explore passing modem by reference

// HTTP(S) Client
TinyGsmClientSecure client(modem);
HttpClient http(client, UPDATE_HOST, 443);

// Authentication
OAuth2 authHandler(OAUTH_HOST, OAUTH_TOKEN_PATH);


void makeGSMConnection()
{
  if (network.connectNetwork()) {
      Serial.println("[+] Connection to mobile network OK");
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
 * @return bool
 */ 
bool setupWifiConnection()
{
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);

  setupSIM800L_GPIO();

  Serial.println("[+] Initializing modem...");

  network.initSim(SIM_PIN);
}

// // Function to update firmware incrementally
// // Buffer is declared to be 128 so chunks of 128 bytes
// // from firmware is written to device until server closes
void updateFirmware(uint8_t *data, size_t len)
{
  Update.write(data, len);
  currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if (currentLength != totalLength)
    return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  // Restart ESP32 to see changes
  ESP.restart();
}

/**
 * @brief Connects via mobile network to Draper Biovillam API to collect auth token before
 * reading DHT sensor and sending all latest readings.  Resets counters.
 *
 */
void connectServer()
{
  std::string completeResponse = "";
  std::string accessToken = "";

  makeGSMConnection();

  Serial.printf("[+] Connected to APN: ", APN);
  Serial.printf("[+] Connecting to ", SERVER);

  if (!client.connect(SERVER, PORT)) {
    Serial.println("[-] Connecting to server failed");
  } else {
    Serial.println("[+] Performing HTTP POST request to OAuth Server");

    client.print(authHandler.personalAccessClientTokenRequestString().c_str()); // better handling of c_str()

    Serial.printf("[D] Request string to get token: ", authHandler.personalAccessClientTokenRequestString(), "\n");

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
    Serial.printf("[+] Access token: ", accessToken, "\n");

    timeout = millis();
  }
}

// Double check both HTTP Client client libs, danger of blending here!
// See if HttpClient has a GET method!
void beginFirwareUpdate() {
  makeGSMConnection();

  if (!client.connect(UPDATE_HOST, PORT)) {
    Serial.println("[-] Connecting to update server failed");
  } else {
    Serial.println("[+] Connected to update server");
  }

  Serial.print(F("[i] Performing HTTPS GET request... "));

  http.connectionKeepAlive();  // Currently, this is needed for HTTPS
  int err = http.get(UPDATE_URL);

  if (err != 0) {
    Serial.println(F("failed to connect"));
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  Serial.println(F("Response Headers:"));
  while (http.headerAvailable()) {
    String headerName  = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    Serial.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    Serial.print(F("Content length is: "));
    Serial.println(length);
  }
  if (http.isResponseChunked()) {
    Serial.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  Serial.println(F("Response:"));
  Serial.println(body);

  Serial.print(F("Body length is: "));
  Serial.println(body.length());

  // If file is reachable, start downloading
  if (resp == 200) {
    // get length of document (is -1 when Server sends no Content-Length header)
    totalLength = client
    totalLength = client.getSize();
    // transfer to local variable
    int len = totalLength;
    // this is required to start firmware update process
    Update.begin(UPDATE_SIZE_UNKNOWN);
    Serial.printf("FW Size: %u\n", totalLength);
    // create buffer for read
    uint8_t buff[128] = {0};
    // get tcp stream
    WiFiClient *stream = client.getStreamPtr();
    // read all data from server
    Serial.println("Updating firmware...");
    while (client.connected() && (len > 0 || len == -1)) {
      // get available data size
      size_t size = stream->available();
      if (size) {
        // read up to 128 byte
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        // pass to function
        updateFirmware(buff, c);
        if (len > 0) {
          len -= c;
        }
      }
      delay(1);
    }
  } else {
    Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
  }
  client.stop();
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