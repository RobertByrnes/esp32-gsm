# 1 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
# 1 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
# 2 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp" 2
# 3 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp" 2

// Constructor
CellularNetwork::CellularNetwork(const char *apn, const char *gprs_user, const char *gprs_password, TinyGsm modem):
  apn(apn), gprs_user(gprs_user), gprs_password(gprs_password), connection(modem) {}

// Destructor
CellularNetwork::~CellularNetwork() {}

/**

 * @brief Make up to 5 attempts to connect to the mobile network.

 * 

 * @param simPin const char *

 * 

 * @return bool

 */
# 18 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
bool CellularNetwork::initSim(const char *simPin) {
   // Set GSM module baud rate and UART pins
  Serial1.begin(115200, 0x800001c, 26 /* Receive pin*/, 27 /* Transmit pin*/);
  delay(3000);

  this->connection.restart();

  // Unlock SIM card with a PIN if needed
  if (strlen(simPin) && this->connection.getSimStatus() != 3) {
    this->connection.simUnlock(simPin);
  }

  return true;
}

/**

 * @brief Make up to 5 attempts to connect to the mobile network.

 * 

 * @return bool

 */
# 38 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
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
# 1 "c:\\code\\toit-sim800l-1\\main.ino"
# 2 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 3 "c:\\code\\toit-sim800l-1\\main.ino" 2

# 5 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 6 "c:\\code\\toit-sim800l-1\\main.ino" 2
// #include <HTTPClient.h>
# 8 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 9 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 10 "c:\\code\\toit-sim800l-1\\main.ino" 2


// Modem
TinyGsm modem(Serial1);
CellularNetwork network(APN, GPRS_USER, GPRS_PASSWORD, modem); // explore passing modem by reference

// HTTP(S) Client
TinyGsmClientSecure client(modem);
HttpClient http(client, HOST, 443);

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
# 38 "c:\\code\\toit-sim800l-1\\main.ino"
void setupSIM800L_GPIO()
{
  pinMode(4 /* Enable pin*/, 0x02);
  pinMode(5 /* Reset pin*/, 0x02);
  pinMode(23 /* Power pin*/, 0x02);
  digitalWrite(4 /* Enable pin*/, 0x0);
  digitalWrite(5 /* Reset pin*/, 0x1);
  digitalWrite(23 /* Power pin*/, 0x1);
}

/**

 * @brief Start WiFi connection.

 * 

 * @return bool

 */
# 53 "c:\\code\\toit-sim800l-1\\main.ino"
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
  Serial.begin(115200);

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
# 98 "c:\\code\\toit-sim800l-1\\main.ino"
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
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("[+] Server disconnected")))));

    network.connection.gprsDisconnect();
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("[+] GPRS disconnected")))));

    accessToken = authHandler.getToken(completeResponse);
    Serial.printf("[+] Access token: ", accessToken, "\n");

    timeout = millis();
  }
}

void beginFirwareUpdate() {
  makeGSMConnection();

  if (!client.connect("http://www.globalrbdev.uk", PORT)) {
    Serial.println("[-] Connecting to update server failed");
  } else {
    Serial.println("[+] Connected to update server");
  }

  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("[i] Performing HTTPS GET request... ")))));

  http.connectionKeepAlive(); // Currently, this is needed for HTTPS
  int err = http.get("/bin/main.ino.bin");

  if (err != 0) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("failed to connect")))));
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Response status code: ")))));
  Serial.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Response Headers:")))));
  while (http.headerAvailable()) {
    String headerName = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    Serial.println("    " + headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Content length is: ")))));
    Serial.println(length);
  }
  if (http.isResponseChunked()) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("The response is chunked")))));
  }

  String body = http.responseBody();
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Response:")))));
  Serial.println(body);

  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Body length is: ")))));
  Serial.println(body.length());

  // If file is reachable, start downloading
  if (resp == 200) {
    // get length of document (is -1 when Server sends no Content-Length header)
    totalLength = client
    totalLength = client.getSize();
    // transfer to local variable
    int len = totalLength;
    // this is required to start firmware update process
    Update.begin(0xFFFFFFFF);
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
  if (millis() - PREVIOUS_MILLIS >= 60000 /* every 1min*/) {
    PREVIOUS_MILLIS = millis();
    RESTART_COUNTER++;
    connectServer();
    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }
}
