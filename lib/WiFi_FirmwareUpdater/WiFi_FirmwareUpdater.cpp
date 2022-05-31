#include "WiFi_FirmwareUpdater.h"


// Constructor
WiFi_FirmwareUpdater::WiFi_FirmwareUpdater(const char* ssid, const char* password):
   ssid(ssid), 
   password(password), 
   totalLength(0), 
   currentLength(0) {}

// Destructor
WiFi_FirmwareUpdater::~WiFi_FirmwareUpdater() {}


void WiFi_FirmwareUpdater::connectWifi() {
    // Start WiFi connection
    WiFi.mode(WIFI_MODE_STA);        
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("[+] WiFi connected");
    Serial.print("[i] Local IP: ");
    Serial.println(WiFi.localIP());
}

/**
 * @brief Function to update firmware incrementally
 * Buffer is declared to be 128 so chunks of 128 bytes
 * from firmware is written to device until server closes.
 * 
 * @param data uint8_t *
 * @param len size_t
 * 
 * @return void
 */ 
void WiFi_FirmwareUpdater::processUpdate(uint8_t *data, size_t len) // private
{
  Update.write(data, len);
  this->currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if (this->currentLength != totalLength) return;
  Update.end(true);
  Serial.printf("\n[+] Update Success, Total Size: %u\nRebooting...\n", this->currentLength); 
  WiFi.disconnect();
  // Restart ESP32 to see changes
  ESP.restart();
}

void WiFi_FirmwareUpdater::getRequest(const char *url)
{
  this->begin(url); // Connect to update server
  this->respCode = this->GET();
  Serial.print("[i] Response: ");
  Serial.println(respCode);
}

void WiFi_FirmwareUpdater::updateFirmware(const char *updateUrl)
{
  this->connectWifi();
  this->getRequest(updateUrl);

  if (this->respCode == 200) {
      int len = totalLength = this->getSize(); // get length of doc (is -1 when Server sends no Content-Length header)
      Update.begin(UPDATE_SIZE_UNKNOWN);
      Serial.printf("[i] Firmware Size: %u\n",totalLength);
      uint8_t buff[128] = { 0 }; // create buffer for read
      WiFiClient * stream = this->getStreamPtr(); // get tcp stream
      Serial.println("[i] Updating firmware...");
      
      while(this->connected() && (len > 0 || len == -1)) { // read all data from server
        size_t size = stream->available(); // get available data size
        if (size) {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size)); // read up to 128 byte
          processUpdate(buff, c);

          if (len > 0) {
              len -= c;
          }
        }
        delay(1);
      }
  } else {
    Serial.println("[-] Cannot download firmware file. Only HTTP response 200 is supported.");
  }
  this->end();
}

bool WiFi_FirmwareUpdater::checkUpdateAvailable(const char *verisonFileUrl)
{
  this->connectWifi();
  this->getRequest(verisonFileUrl);

  if (this->respCode == 200) {
    int len = totalLength = this->getSize(); // get length of doc (is -1 when Server sends no Content-Length header) 
    uint8_t buff[128] = { 0 }; // create buffer for read
    WiFiClient * stream = this->getStreamPtr(); // get tcp stream
    Serial.println("[i] Checking firmware version...");
    String version = this->getString().substring(version.lastIndexOf("=") + 1); // grrrrr had to use String
    Serial.printf("[i] Firmware Version available: %s\n", version);
  } else {
    Serial.println("[-] Cannot download firmware version file. Only HTTP response 200 is supported.");  
    this->end(); 
    WiFi.disconnect();
    return false;
  }
  
  this->end(); 
  WiFi.disconnect();
  return true;
}
