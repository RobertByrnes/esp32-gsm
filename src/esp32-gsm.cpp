#include <Arduino.h>
#include <DataUploadApi.h>
#include <WiFi_FirmwareUpdater.h>
#include "config.h"
#include <ESP32TimerInterrupt.h>

#ifdef WIFI_UPDATES
  #include <WiFi_FirmwareUpdater.h>
#else
  #include <GSMFirmwareUpdater.h>
#endif

// User queues
#define LED_PIN 13

// Memory profiling - uncomment to enable
#define PROFILE_MEMORY

// Serial print verbosity - uncomment to enable output (it's a good idea to turn this off in production, unless Serial connection is available)
#define SERIAL_VERBOSE true

// ESP hardware timers
hw_timer_t *timer_1 = NULL; // to run on core 0 - mission critical timed events to run on core 0
hw_timer_t *timer_2 = NULL; // to run on core 1 - all things GSM / WIFI / HTTP(s)
portMUX_TYPE timerMux_1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux_2 = portMUX_INITIALIZER_UNLOCKED;
volatile int interruptCounter_1; // volatile as being shared between ISR in IRAM and main in FLASH
volatile int interruptCounter_2;
volatile int interruptCounter_3;

// Multicore tasking
TaskHandle_t task_1; // to run on core 0
TaskHandle_t task_2; // to run on core 1

// Modem
TinyGsm modem(SerialAT);
CellularNetwork800L network(APN, GPRS_USER, GPRS_PASSWORD, modem);

// HTTP(S) Client
TinyGsmClientSecure client(modem);

// Authentication
DataUploadApi api(network, client, OAUTH_HOST, OAUTH_TOKEN_PATH);

// Firmware Updates
#ifdef WIFI_UPDATES
WiFi_FirmwareUpdater update(SSID, PASSWORD, CURRENT_VERSION);
#else
GSMFirmwareUpdater update(UPDATE_URL, UPDATE_HOST, PORT);
#endif

#ifdef PROFILE_MEMORY
void memoryProfile(std::string taskHandle, TaskHandle_t &task)
{
#ifdef SERIAL_VERBOSE
  Serial.println("\n*****************************************");
  Serial.printf("App Size: %u\n", ESP.getSketchSize());
  Serial.printf("Free App Space: %u\n", ESP.getFreeSketchSpace());
  Serial.printf("Psram Size: %u\n", ESP.getPsramSize()); // Psuedo Static RAM is enable in the build flags
  Serial.printf("Free Psram: %u\n", ESP.getFreePsram());
  Serial.printf("Heap Size: %u\n", ESP.getHeapSize()); 
  Serial.printf("Free Heap: %u\n", ESP.getFreeHeap());
  Serial.print(String(taskHandle.c_str()) + " stack not used: ");
  Serial.println(uxTaskGetStackHighWaterMark(task));
  Serial.println("*****************************************");
#endif
}
#endif


/**
 * @brief Use the OAuth2 class to get formatted HTTP string to send a POST
 * request to the OAuth server. The token is saved to a member variable
 * of the OAuth2 class class currentToken.
 * 
 * @return void
 */
void getOAuthToken(const char *apn, const char *server, const uint16_t &port)
{
  std::string completeResponse = "";
  std::string accessToken = "";

  if (!network.connectNetwork()) {
#ifdef SERIAL_VERBOSE
    Serial.println("[-] failed to connect to mobile network");
#endif
    return; // better handling if it falls over
  }

#ifdef SERIAL_VERBOSE
  Serial.println("[+] Connected to mobile network OK");
  Serial.print("[+] Connected to APN: ");
  Serial.println(apn);
  Serial.print("[+] Connecting to ");
  Serial.println(server);
#endif

  if (!client.connect(server, port)) {
#ifdef SERIAL_VERBOSE
    Serial.println("[-] Connecting to server failed");
#endif
    return; // better handling if it falls over
  } else {
#ifdef SERIAL_VERBOSE
    Serial.println("[+] Performing HTTP POST request to OAuth Server");
    Serial.print("[D] Request string to get token: ");
    Serial.println(api.personalAccessClientTokenRequestString());
#endif
    client.print(api.personalAccessClientTokenRequestString());

    unsigned long timeout = millis();
    char response;

    while (client.connected() && millis() - timeout < 10000L) {
      while (client.available()) {
        char response = client.read();
        completeResponse += response;
      }
    }

    client.stop();
    accessToken = api.getToken(completeResponse);

#ifdef SERIAL_VERBOSE
    Serial.println(F("[+] Server disconnected"));
    Serial.print("[D] Response: ");
    Serial.println(completeResponse.c_str());
    Serial.print("[+] Access token: ");
    Serial.println(accessToken.c_str());
#endif

    network.connection.gprsDisconnect();
#ifdef SERIAL_VERBOSE
    Serial.println(F("[+] GPRS disconnected"));
#endif
  }
}

/**
 * @brief Interrupt service routine callback func,
 * increment counter shared between IRAM and main.
 * Runs on core 0.
 * 
 * @return void
 */
void IRAM_ATTR core_0_ISR_1()
{
  portENTER_CRITICAL_ISR(&timerMux_1);
  interruptCounter_1++;
  portEXIT_CRITICAL_ISR(&timerMux_1);
}

/**
 * @brief Interrupt service routine callback func,
 * increment counter shared between IRAM and main.
 * Runs on core 1 with main application.
 * The first ISR on core 1. 
 * 
 * @return void
 */
void IRAM_ATTR core_1_ISR_1()
{
  portENTER_CRITICAL_ISR(&timerMux_2);
  interruptCounter_2++;
  interruptCounter_3++;
  portEXIT_CRITICAL_ISR(&timerMux_2);
}

/**
 * @brief Core 0 - mission critical timed events.
 * 
 * @return void
 */
static void core_0_task_1(void *pvParameters)
{
  Serial.print("[i] Task_1 running on: core ");
  Serial.println( xPortGetCoreID() );

  // ISR 1
  timer_1 = timerBegin(0, 80, true); // timer_no / prescaler / countup
  timerAttachInterrupt(timer_1, &core_0_ISR_1, false); // boolean is for edge / level
  timerAlarmWrite(timer_1, TIMER_1_CONNECTION_INTERVAL, true); // 10sec
  timerAlarmEnable(timer_1);

  for (;;) {
    if (interruptCounter_1 > 0) {
#ifdef PROFILE_MEMORY
      // memoryProfile("task_1", task_1);
#endif
      portENTER_CRITICAL(&timerMux_1);
      interruptCounter_1--;
      portEXIT_CRITICAL(&timerMux_1);

      int i = 0;
      while (i < 2) {
        digitalWrite(LED_PIN, HIGH);
        sleep(1);
        digitalWrite(LED_PIN, LOW);
        sleep(1);
        i++;
      }
    }
  }
}

/**
 * @brief Core 1 - HTTP(s) tasks.
 * 
 * @return void
 */
static void core_1_task_1(void *pvParameters)
{
  Serial.print("[i] Task_2 running on: core ");
  Serial.println( xPortGetCoreID() );

  if (!SPIFFS.begin(false)) {
    Serial.println("[i] SPIFFS Mount Failed");
  }
  
  Serial.println("[i] SPIFFS Mounted, formatting...");
  // SPIFFS.format();
  Serial.println("[i] SPIFFS Formatted");
  update.listDir(SPIFFS, "/", 0);
  delay(5);

  // ISR 2
  timer_2 = timerBegin(1, 80, true); // timer_no / prescaler / countup
  timerAttachInterrupt(timer_2, &core_1_ISR_1, false); // boolean is for edge / level
  timerAlarmWrite(timer_2, TIMER_2_CONNECTION_INTERVAL, true); // 1min
  timerAlarmEnable(timer_2);

  for (;;) {
    if (interruptCounter_2 > 10) {
#ifdef PROFILE_MEMORY
      memoryProfile("task_2 - oauth", task_2);
#endif
      portENTER_CRITICAL(&timerMux_2);
      interruptCounter_2--;
      portEXIT_CRITICAL(&timerMux_2);

      // connect to the OAuth server
      getOAuthToken(APN, SERVER, PORT);
    }

    if (interruptCounter_3 > 0) {
#ifdef PROFILE_MEMORY
      memoryProfile("task_2 - update", task_2);
#endif
      portENTER_CRITICAL(&timerMux_2);
      interruptCounter_3 = 0;
      portEXIT_CRITICAL(&timerMux_2);

#ifdef WIFI_UPDATES
      // check for and perform firmware update
      if (update.checkUpdateAvailable(UPDATE_VERSION_FILE_URL)) {
        // update.updateFirmware(UPDATE_URL);
      }
#endif

#ifndef WIFI_UPDATES
      update.updateFirmware(client, network);
#endif
    }
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);

  pinMode(LED_PIN, OUTPUT);
  // setCpuFrequencyMhz();
  // psramInit();
  // Disable core 0 watchdogs
  disableCore0WDT();

  xTaskCreatePinnedToCore(core_0_task_1, // Task function.
      "Task1", // name of task.
      3000, // Stack size of task
      NULL, // parameter of the task
      1, // priority of the task
      &task_1, // Task handle to keep track of created task
      0 // pin task to core 0
  ); 

  sleep(2);

  xTaskCreatePinnedToCore(core_1_task_1, // Task function.
      "Task2", // name of task.
      15000, // Stack size of task
      NULL, // parameter of the task
      1, // priority of the task
      &task_2, // Task handle to keep track of created task
      1 // pin task to core 1
  );
}

void loop() { vTaskDelete(NULL); }
