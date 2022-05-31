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
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
// Global variables
int totalLength;       
int currentLength = 0; 


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
void WiFi_FirmwareUpdater::updateFirmware(uint8_t *data, size_t len)
{
  Update.write(data, len);
  currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  // Restart ESP32 to see changes 
  ESP.restart();
}

void setup()
{
  Serial.begin(115200);

  // Connect to external web server
  this->begin(UPDATE_URL);
  // Get file, just to check if each reachable
  int resp = client.GET();
  Serial.print("Response: ");
  Serial.println(resp);
  // If file is reachable, start downloading
  if(resp == 200){
      // get length of document (is -1 when Server sends no Content-Length header)
      totalLength = client.getSize();
      // transfer to local variable
      int len = totalLength;
      // this is required to start firmware update process
      Update.begin(UPDATE_SIZE_UNKNOWN);
      Serial.printf("FW Size: %u\n",totalLength);
      // create buffer for read
      uint8_t buff[128] = { 0 };
      // get tcp stream
      WiFiClient * stream = client.getStreamPtr();
      // read all data from server
      Serial.println("Updating firmware...");
      while(client.connected() && (len > 0 || len == -1)) {
           // get available data size
           size_t size = stream->available();
           if(size) {
              // read up to 128 byte
              int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
              // pass to function
              updateFirmware(buff, c);
              if(len > 0) {
                 len -= c;
              }
           }
           delay(1);
      }
  }else{
    Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
  }
  client.end();
  
}

void loop() {}