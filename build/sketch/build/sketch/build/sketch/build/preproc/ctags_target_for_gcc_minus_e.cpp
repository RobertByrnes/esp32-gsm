# 1 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
# 1 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
# 2 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp" 2
# 3 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp" 2

// Constructor
CellularNetwork::CellularNetwork(const char *apn, const char *gprs_user, const char *gprs_password,, const char simPin, TinyGsm modem):
  apn(apn), gprs_user(gprs_user), gprs_password(gprs_password), connection(modem) {
    this->initSim(simPin);
  }

// Destructor
CellularNetwork::~CellularNetwork() {}

bool CellularNetwork::initSim(const char simPin) {
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
# 33 "c:\\code\\toit-sim800l-1\\CellularNetwork.cpp"
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
# 7 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 8 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 9 "c:\\code\\toit-sim800l-1\\main.ino" 2


// INSTANTIATIONS
TinyGsm modem(Serial1);

CellularNetwork network(APN, GPRS_USER, GPRS_PASSWORD, SIM_PIN, modem); // explore passing modem by reference

TinyGsmClientSecure client(modem);

OAuth2 authHandler(OAUTH_HOST, OAUTH_TOKEN_PATH);


void setup()
{
  if (true) Serial.begin(115200);

  // Set modem enable, reset and power pins
  pinMode(4 /* Enable pin*/, 0x02);
  pinMode(5 /* Reset pin*/, 0x02);
  pinMode(23 /* Power pin*/, 0x02);
  digitalWrite(4 /* Enable pin*/, 0x0);
  digitalWrite(5 /* Reset pin*/, 0x1);
  digitalWrite(23 /* Power pin*/, 0x1);


  // Set GSM module baud rate and UART pins
  Serial1.begin(115200, 0x800001c, 26 /* Receive pin*/, 27 /* Transmit pin*/);
  delay(3000);

  if (true) Serial.println("[+] Initializing modem...");

  modem.restart();

  // Unlock SIM card with a PIN if needed
  if (strlen(SIM_PIN) && modem.getSimStatus() != 3) {
    modem.simUnlock(SIM_PIN);
  }



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
# 87 "c:\\code\\toit-sim800l-1\\main.ino"
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

  if (true) Serial.printf("[+] Connecting to ", SERVER);

  if (!client.connect(SERVER, PORT)) {
    if (true) Serial.println("[-] Connecting to server failed");
  } else {
    if (true) Serial.println("[+] Performing HTTP POST request to OAuth Server");

    client.print(authHandler.personalAccessClientTokenRequestString().c_str());

    if (true) Serial.printf("[D] Request string to get token: ", authHandler.personalAccessClientTokenRequestString(), "\n");

    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 10000L) {

      while (client.available()) {

        char response = client.read();

        if (true) {
          Serial.print(response);
        } else {
          Serial.println("[-] Error in response");
        }
        completeResponse.concat(response);
      }
    }

    client.stop();
    modem.gprsDisconnect();

    if (true) {
      Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("[+] Server disconnected")))));
      Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("[+] GPRS disconnected")))));
    }

    String parsedResponse = authHandler.findJson(completeResponse);

    if (parsedResponse != "") {
      accessToken = authHandler.extractToken(parsedResponse);
      if (true) Serial.println("[+] Access token: " + accessToken);
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
  if (millis() - PREVIOUS_MILLIS >= 60000 /* every 1min*/) {
    PREVIOUS_MILLIS = millis();
    RESTART_COUNTER++;
    connectServer();
    if (RESTART_COUNTER >= 5) {
      ESP.restart();
    }
  }
}
