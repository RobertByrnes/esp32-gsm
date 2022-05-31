# 1 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino"
# 1 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino"
/**************************************************************
 *
 * For this example, you need to install CRC32 library:
 *   https://github.com/bakercp/CRC32
 *   or from http://librarymanager/all#CRC32+checksum
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 * ATTENTION! Downloading big files requires of knowledge of
 * the TinyGSM internals and some modem specifics,
 * so this is for more experienced developers.
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800 

#define MODEM_RST 5 /* Reset pin*/
#define MODEM_PWKEY 4 /* Enable pin*/
#define MODEM_POWER_ON 23 /* Power pin*/
#define MODEM_TX 27 /* Transmit pin*/
#define MODEM_RX 26 /* Receive pin*/
#define I2C_SDA 21 /* Serial data*/
#define I2C_SCL 22 /* Serial clock*/

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT Serial1

// Registry config values for I2C
// with the SIM800 modem
#define IP5306_ADDR 0x75 /* Initiate communication*/
#define IP5306_REG_SYS_CTL0 0x00 /* Begin write*/

// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).

#define TINY_GSM_RX_BUFFER 1024


// See all AT commands, if wanted
#define DUMP_AT_COMMANDS 

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING  // <- Logging is for the HTTP library

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "giffgaff.com";
const char gprsUser[] = "gg";
const char gprsPass[] = "p";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// Server details
const char server[] = "vsh.pp.ua";
const int port = 80;

# 79 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino" 2
# 80 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino" 2

// Just in case someone defined the wrong thing..
# 95 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino"
const char resource[] = "/TinyGSM/test_1k.bin";
uint32_t knownCRC32 = 0x6f50d767;
uint32_t knownFileSize = 1024; // In case server does not send it


# 101 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino" 2
StreamDebugger debugger(Serial1, Serial);
TinyGsm modem(debugger);




TinyGsmClient client(modem);

void setup() {
  pinMode(4 /* Enable pin*/, 0x02);
  pinMode(5 /* Reset pin*/, 0x02);
  pinMode(23 /* Power pin*/, 0x02);
  digitalWrite(4 /* Enable pin*/, 0x0);
  digitalWrite(5 /* Reset pin*/, 0x1);
  digitalWrite(23 /* Power pin*/, 0x1);
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  Serial.println("Wait...");

  // Set GSM module baud rate
  Serial1.begin(115200);
  delay(6000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);


  // Unlock your SIM card with a PIN if needed
  if ("" && modem.getSimStatus() != 3) { modem.simUnlock(""); }

}

void printPercent(uint32_t readLength, uint32_t contentLength) {
  // If we know the total length
  if (contentLength != (uint32_t)-1) {
    Serial.print("\r ");
    Serial.print((100.0 * readLength) / contentLength);
    Serial.print('%');
  } else {
    Serial.println(readLength);
  }
}

void loop() {
# 174 "C:\\Users\\robby\\OneDrive\\Desktop\\test-download\\test-download.ino\\test-download.ino.ino"
  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) { Serial.println("Network connected"); }


  // GPRS connection parameters are usually set after network registration
  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Connecting to ")))));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) { Serial.println("GPRS connected"); }


  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Connecting to ")))));
  Serial.print(server);
  if (!client.connect(server, port)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();

  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Waiting for response header")))));

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the client, we need to exit.

  const uint32_t clientReadTimeout = 5000;
  uint32_t clientReadStartTime = millis();
  String headerBuffer;
  bool finishedHeader = false;
  uint32_t contentLength = 0;

  while (!finishedHeader) {
    int nlPos;

    if (client.available()) {
      clientReadStartTime = millis();
      while (client.available()) {
        char c = client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        // if (c < 16)
        //   SerialMon.print('0');
        // SerialMon.print(c, HEX);
        // SerialMon.print(' ');
        // if (isprint(c))
        //   SerialMon.print(reinterpret_cast<char> c);
        // else
        //   SerialMon.print('*');
        // SerialMon.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(((reinterpret_cast<const __FlashStringHelper *>(("\r\n"))))) >= 0) break;
      }
    } else {
      if (millis() - clientReadStartTime > clientReadTimeout) {
        // Time-out waiting for data from client
        Serial.println(((reinterpret_cast<const __FlashStringHelper *>((">>> Client Timeout !")))));
        break;
      }
    }

    // See if we have a new line.
    nlPos = headerBuffer.indexOf(((reinterpret_cast<const __FlashStringHelper *>(("\r\n")))));

    if (nlPos > 0) {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(((reinterpret_cast<const __FlashStringHelper *>(("content-length:")))))) {
        contentLength =
            headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        // SerialMon.print(F("Got Content Length: "));  // uncomment for
        // SerialMon.println(contentLength);            // confirmation
      }

      headerBuffer.remove(0, nlPos + 2); // remove the line
    } else if (nlPos == 0) {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line),
      // we are done with the header.
      finishedHeader = true;
    }
  }

  // The two cases which are not managed properly are as follows:
  // 1. The client doesn't provide data quickly enough to keep up with this
  // loop.
  // 2. If the client data is segmented in the middle of the 'Content-Length: '
  // header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32 crc;

  if (finishedHeader && contentLength == knownFileSize) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Reading response data")))));
    clientReadStartTime = millis();

    printPercent(readLength, contentLength);
    while (readLength < contentLength && client.connected() &&
           millis() - clientReadStartTime < clientReadTimeout) {
      while (client.available()) {
        uint8_t c = client.read();
        // SerialMon.print(reinterpret_cast<char>c);  // Uncomment this to show
        // data
        crc.update(c);
        readLength++;
        if (readLength % (contentLength / 13) == 0) {
          printPercent(readLength, contentLength);
        }
        clientReadStartTime = millis();
      }
    }
    printPercent(readLength, contentLength);
  }

  timeElapsed = millis() - timeElapsed;
  Serial.println();

  // Shutdown

  client.stop();
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Server disconnected")))));






  modem.gprsDisconnect();
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("GPRS disconnected")))));


  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");
  Serial.println(contentLength);
  Serial.print("Actually read:  ");
  Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x");
  Serial.println(crc.finalize(), 16);
  Serial.print("Known CRC32:    0x");
  Serial.println(knownCRC32, 16);
  Serial.print("Duration:       ");
  Serial.print(duration);
  Serial.println("s");

  // Do nothing forevermore
  while (true) { delay(1000); }
}
