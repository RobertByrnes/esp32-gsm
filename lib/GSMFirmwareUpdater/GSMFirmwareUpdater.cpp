#include "GSMFirmwareUpdater.h"


// Constructor
GSMFirmwareUpdater::GSMFirmwareUpdater(): totalLength(0), currentLength(0) {}

// Destructor
GSMFirmwareUpdater::~GSMFirmwareUpdater() {}



void GSMFirmwareUpdater::printPercent(uint32_t readLength, uint32_t contentLength) {
  // If we know the total length
  if (contentLength != (uint32_t)-1) {
    Serial.print("\r ");
    Serial.print((100.0 * readLength) / contentLength);
    Serial.print('%');
  } else {
    Serial.println(readLength);
  }
}

/**
 * @brief Function to update firmware incrementally.
 * Buffer is declared to be 128 so chunks of 128 bytes
 * from firmware is written to device until server closes.
 * 
 * @param UPDATE_HOST const char *
 * @param PORT uint16_t
 * @param http HttpClient
 * @param client TinyGsmClientSecure
 * 
 * @return void
 */ 
void GSMFirmwareUpdater::performUpdate(const char *UPDATE_URL, const char *UPDATE_HOST, const uint16_t PORT, CRC32 &crc, TinyGsmClientSecure &client, CellularNetwork &network)
{  
  uint32_t   knownCRC32    = 0x6f50d767;
  uint32_t   knownFileSize = 1024;  // In case server does not send it

  if (!client.connect(UPDATE_HOST, PORT)) {
    Serial.print("[-] Connecting to update server failed: Url\n");
    Serial.println(UPDATE_HOST);
    Serial.println(PORT);
    Serial.print(UPDATE_URL);

    return;
    
  } else {
    Serial.println("[+] Connected to update server");
  }

  Serial.print("[i] Performing HTTPS GET request to ");
  Serial.println(UPDATE_URL);

  // Make a HTTP GET request:
  client.print(String("GET ") + UPDATE_URL + " HTTP/1.0\r\n");
  client.print(String("Host: ") + UPDATE_HOST + "\r\n");
  client.print("Connection: close\r\n\r\n");
  Serial.println(F("[i] Waiting for response header"));
  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the client, we need to exit.

  const uint32_t clientReadTimeout   = 300000;
  uint32_t       clientReadStartTime = millis();
  String         headerBuffer;
  bool           finishedHeader = false;
  uint32_t       contentLength  = 0;

  while (!finishedHeader) {
    int nlPos;

    if (client.available()) {
      clientReadStartTime = millis();
      while (client.available()) {
        char c = client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        if (c < 16)
          Serial.print('0');
        Serial.print(c, HEX);
        Serial.print(' ');
        if (isprint(c))
          Serial.print(reinterpret_cast<char>(c));
        else
          Serial.print('*');
        Serial.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0) break;
      }
    } else {
      if (millis() - clientReadStartTime > clientReadTimeout) {
        // Time-out waiting for data from client
        Serial.println(F(">>> Client Timeout !"));
        break;
      }
    }

    // See if we have a new line.
    nlPos = headerBuffer.indexOf(F("\r\n"));

    if (nlPos > 0) {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(F("content-length:"))) {
        contentLength = headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        Serial.print(F("Got Content Length: "));  // uncomment for
        Serial.println(contentLength);            // confirmation
      }

      headerBuffer.remove(0, nlPos + 2);  // remove the line
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

  if (finishedHeader && contentLength == knownFileSize) {
    Serial.println(F("Reading response data"));
    clientReadStartTime = millis();

    printPercent(readLength, contentLength);
    while (readLength < contentLength && client.connected() &&
           millis() - clientReadStartTime < clientReadTimeout) {
      while (client.available()) {
        uint8_t c = client.read();
        Serial.print(c);  // Uncomment this to show
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
  Serial.println(F("Server disconnected"));

// #if TINY_GSM_USE_WIFI
//   modem.networkDisconnect();
//   Serial.println(F("WiFi disconnected"));
// #endif
// #if TINY_GSM_USE_GPRS
  network.connection.gprsDisconnect();
  Serial.println(F("GPRS disconnected"));
// #endif

  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");
  Serial.println(contentLength);
  Serial.print("Actually read:  ");
  Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x");
  Serial.println(crc.finalize(), HEX);
  Serial.print("Known CRC32:    0x");
  Serial.println(knownCRC32, HEX);
  Serial.print("Duration:       ");
  Serial.print(duration);
  Serial.println("s");

  // Do nothing forevermore
  // while (true) { delay(1000); }
}

void GSMFirmwareUpdater::updateFirmware(uint8_t *data, size_t len) {
  size_t written = Update.write(data, len);
  Serial.print("Written: ");
  Serial.println(written);
  this->currentLength += len;
  Serial.print("Current Length: ");
  Serial.println(currentLength);
  Serial.print("Total Length: ");
  Serial.println(totalLength);
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if (this->currentLength != this->totalLength)
    return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", this->currentLength);
  // Restart ESP32 to see changes
  ESP.restart();
}
