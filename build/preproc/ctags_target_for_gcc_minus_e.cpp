# 1 "c:\\code\\toit-sim800l-1\\config.h"
# 1 "c:\\code\\toit-sim800l-1\\config.h"
// This uncommented, enables debugging
// output via the serial monitor
#define DEV true


// Aliases Serial interface for writing to the debugging
// console, with a default baud rate of 115200
#define Debugger Serial


// GPRS credentials
const char apn[] = "giffgaff.com"; // APN Access Poing Name
const char gprsUser[] = "gg"; // GPRS User
const char gprsPass[] = "p"; // GPRS Password

// SIM card PIN, leave empty if not defined
const char simPIN[] = "";

// End point details for the server
const char server[] = "draperbiotech.clystnet.com"; // Base url
const char authPath[] = "/oauth/token"; // oAuth token path
const char resourcePath[] = "/api/readings"; // Resource path
const int port = 443; // Server port number

// End point details for the API
const char udid[] = "Proto2";
const char clientSecret[] = "RN6aJMOXOOSKcNBND2YOKuUQMKlKATgNu3iSYoHn";
String grantType = "client_credentials";
const int clientID = 6;

// SIM800 pins
#define MODEM_RST 5 /* Reset pin*/
#define MODEM_PWKEY 4 /* Enable pin*/
#define MODEM_POWER_ON 23 /* Power pin*/
#define MODEM_TX 27 /* Transmit pin*/
#define MODEM_RX 26 /* Receive pin*/
#define I2C_SDA 21 /* Serial data*/
#define I2C_SCL 22 /* Serial clock*/

// Aliases Serial1 interface for AT 
// commands to SIM800 module
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800 /* Modem is SIM800*/
#define TINY_GSM_RX_BUFFER 1024 /* Set RX buffer to 1Kb*/

// Registry config values for I2C
// with the SIM800 modem
#define IP5306_ADDR 0x75 /* Initiate communication*/
#define IP5306_REG_SYS_CTL0 0x00 /* Begin write*/

#define CONNECTION_INTERVAL 900000
#define CONNECTION_INTERVAL 3000
// ESP restart counter
unsigned int restart = 0;

// Counters
unsigned long augerPreviousMillis = 0;
unsigned long commsPreviousMillis = 0;
unsigned long faultPreviousMillis = 0;
const unsigned int augerPollingInterval = 700;
const unsigned int faultPollingInterval = 30000;

// location of firmware file on external web server
// change to your actual .bin location
#define HOST "http://www.globalrbdev.uk/bin/main.ino.bin"

// Your WiFi credentials
const char* ssid = "CrowdedHouse";
const char* password = "kF4QMhzc3xcS";
// Global variables
int totalLength; //total size of firmware
int currentLength = 0; //current size of written firmware
# 1 "c:\\code\\toit-sim800l-1\\main.ino"
# 2 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 3 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 4 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 5 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 6 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 7 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 8 "c:\\code\\toit-sim800l-1\\main.ino" 2
# 9 "c:\\code\\toit-sim800l-1\\main.ino" 2

// INSTANTIATIONS
// #define DUMP_AT_COMMANDS
// Define the serial console for debug prints




TinyGsm modem(Serial1);


// Modem
TinyGsmClientSecure client(modem);

// WiFi
// HTTPClient client;

void setup()
{
  if (true)
    Serial.begin(115200);

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

  if (true)
    Serial.println("[+] Initializing modem...");

  modem.restart();

  // Unlock SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3)
    modem.simUnlock(simPIN);


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

void loop()
{
  if (millis() - commsPreviousMillis >= 3000) {
    commsPreviousMillis = millis();
    restart++;
    connectServer();
    if (restart >= 5) {
      ESP.restart();
    }
  }
}

// Function to update firmware incrementally
// Buffer is declared to be 128 so chunks of 128 bytes
// from firmware is written to device until server closes
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

 * @brief returns to correct string for requesting an auth token from the server

 *

 * @return String

 */
# 103 "c:\\code\\toit-sim800l-1\\main.ino"
String httpRequestAuthString()
{
  return "grant_type=" + grantType + "&client_id=" + clientID + "&client_secret=" + clientSecret + "";
}

/**

 * @brief Connects via mobile network to Draper Biovillam API to collect auth token before

 * reading DHT sensor and sending all latest readings.  Resets counters.

 *

 */
# 113 "c:\\code\\toit-sim800l-1\\main.ino"
void connectServer()
{
  String completeResponse = "";
  String accessToken = "";

  if (true) {
    Serial.print("[+] Connecting to APN: ");
    Serial.println(apn);
  }

  connectApn();

  if (true) {
    Serial.print("[+] Connecting to ");
    Serial.println(server);
  }

  if (!client.connect(server, port)) {
    if (true) {
      Serial.println("[-] Connecting to server failed");
    }
  } else {
    if (true) {
      Serial.println(" OK");
      Serial.println("[+] Performing HTTP POST request to API to request an Access Token with:\n");
    }

    client.print(String("POST ") + authPath + " HTTP/1.1\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.print(String("Accept: application/json\r\n"));
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print(String("Content-Length: ") + httpRequestAuthString().length() + "\r\n\r\n");
    client.println(httpRequestAuthString());

    if (true) {
      Serial.println(httpRequestAuthString());
      Serial.print(String("POST ") + authPath + " HTTP/1.1\r\n");
      Serial.print(String("Host: ") + server + "\r\n");
      Serial.print(String("Accept: application/json\r\n"));
      Serial.print("Content-Type: application/x-www-form-urlencoded\r\n");
      Serial.print(String("Content-Length: ") + httpRequestAuthString().length() + "\r\n\r\n");
    }

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

    String parsedResponse = findJson(completeResponse);

    if (parsedResponse != "") {
      accessToken = extractToken(parsedResponse);
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

void connectApn()
{
    // Make up to 5 attempts to connect to the mobile network
  int i = 0;
  while (i <= 5) {
    ++i;
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      if (true) Serial.println("[-] failed to connect to mobile network");
      delay(3000);
      if (i == 5) continue;
    } else {
      Serial.println("[+] Connection to mobile network OK");
      break;
    }
  }
}
/**

 * @brief Extract JSON from server response

 * 

 * @param response 

 * @return String 

 */
# 263 "c:\\code\\toit-sim800l-1\\main.ino"
String findJson(String response)
{
  Serial.println(response);
  int firstCurly = response.indexOf("{") + 1;
  int secondCurly = response.indexOf("}");
  // String JSON = "";
  String JSON = response.substring(firstCurly, secondCurly);

  // Serial.println(JSON);
  if (JSON.length() > 1)
    return JSON;
  else
    return String("");
}

/**

 * @brief Extract auth token from JSON string

 * 

 * @param JSON 

 * @return String 

 */
# 284 "c:\\code\\toit-sim800l-1\\main.ino"
String extractToken(String JSON)
{
  int beginningOfToken = JSON.lastIndexOf(":") + 2;
  int endOfToken = JSON.lastIndexOf("\"");
  String accesToken = JSON.substring(beginningOfToken, endOfToken);
  Serial.println("[+] Acces Token: " + accesToken);

  if (accesToken.length() > 1)
    return accesToken;
  else
    return String("");
}
