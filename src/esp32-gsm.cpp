#include <Arduino.h>
#include <DataUploadApi.h>
#include "WiFi_FirmwareUpdater.h"
#include "config.h"
#include <ESP32TimerInterrupt.h>
#include <SimpleTimer.h>   

// Timer demonstration
#define LED_PIN 13

// ESP hardware timers
hw_timer_t *timer_1 = NULL; // to run on core 0 - mission critical timed events to run on core 0
hw_timer_t *timer_2 = NULL; // to run on core 1 - all things GSM / WIFI / HTTP(s)
hw_timer_s *timer_3 = NULL; // to run on core 1 - all things GSM / WIFI / HTTP(s)
portMUX_TYPE timerMux_1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux_2 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux_3 = portMUX_INITIALIZER_UNLOCKED;
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
WiFi_FirmwareUpdater update(SSID, PASSWORD, CURRENT_VERSION);

void memoryProfile(std::string taskHandle, TaskHandle_t &task) {
  Serial.println("*******************************");
  Serial.printf("App Size: %u\n", ESP.getSketchSize());
  Serial.printf("Free App Space: %u\n", ESP.getFreeSketchSpace());
  Serial.printf("Psram Size: %u\n", ESP.getPsramSize());
  Serial.printf("Free Psram: %u\n", ESP.getFreePsram());
  Serial.printf("Heap Size: %u\n", ESP.getHeapSize()); 
  Serial.printf("Free Heap: %u\n", ESP.getFreeHeap());
  Serial.print(String(taskHandle.c_str()) + "task_1, stack not used: ");
  Serial.println(uxTaskGetStackHighWaterMark(task));
  Serial.println("*******************************");
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
  portEXIT_CRITICAL_ISR(&timerMux_2);
}

/**
 * @brief Interrupt service routine callback func,
 * increment counter shared between IRAM and main.
 * Runs on core 1 with main application.
 * The second ISR on core 1.
 * 
 * @return void
 */
void IRAM_ATTR core_1_ISR_2()
{
  portENTER_CRITICAL_ISR(&timerMux_3);
  interruptCounter_3++;
  portEXIT_CRITICAL_ISR(&timerMux_3);
}

/**
 * @brief Core 0 - mission critical timed events.
 * 
 * @return void
 */
static void core_0_task(void *pvParameters)
{
  Serial.print("[i] Task_1 running on: core ");
  Serial.println( xPortGetCoreID() );

  // ISR 1
  timer_1 = timerBegin(0, 80, true); // timer_no / prescaler / countup
  timerAttachInterrupt(timer_1, &core_0_ISR_1, true); // boolean is for edge / level
  timerAlarmWrite(timer_1, 10000000, true); // 10sec
  timerAlarmEnable(timer_1);

  for (;;) {
    if (interruptCounter_1 > 0) {
      memoryProfile("task_1", task_1);
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
static void core_1_task(void *pvParameters)
{
  Serial.print("[i] Task_2 running on: core ");
  Serial.println( xPortGetCoreID() );

  // ISR 2
  timer_2 = timerBegin(1, 80, true); // timer_no / prescaler / countup
  timerAttachInterrupt(timer_2, &core_1_ISR_1, true); // boolean is for edge / level
  timerAlarmWrite(timer_2, TIMER_2_CONNECTION_INTERVAL, true); // 1min
  timerAlarmEnable(timer_2);

  // ISR 3
  timer_3 = timerBegin(2, 80, true); // timer_no / prescaler / countup
  timerAttachInterrupt(timer_3, &core_1_ISR_2, true); // boolean is for edge / level
  timerAlarmWrite(timer_3, TIMER_3_CONNECTION_INTERVAL, true); // 2mins
  timerAlarmEnable(timer_3);

  for (;;) {
    if (interruptCounter_2 > 0) {
      memoryProfile("task_2", task_2);
      portENTER_CRITICAL(&timerMux_2);
      interruptCounter_2--;
      portEXIT_CRITICAL(&timerMux_2);

      // connect to the OAuth server
      api.connectServer(APN, SERVER, PORT);
    }
    

    // if (interruptCounter_3 > 0) {
    //   portENTER_CRITICAL(&timerMux_3);
    //   interruptCounter_3--;
    //   portEXIT_CRITICAL(&timerMux_3);

    //   // check for and perform firmware update
    //   if (update.checkUpdateAvailable(UPDATE_VERSION_FILE_URL)) {
    //     update.updateFirmware(UPDATE_URL);
    //   }
    // }
  }
}


void setup()
{
  Serial.begin(BAUD_RATE);

  pinMode(LED_PIN, OUTPUT);

  // Disable core 0 watchdogs
  disableCore0WDT();

  xTaskCreatePinnedToCore(core_0_task, // Task function.
      "Task1", // name of task.
      10000, // Stack size of task
      NULL, // parameter of the task
      1, // priority of the task
      &task_1, // Task handle to keep track of created task
      0 // pin task to core 0
  ); 

  sleep(2);

  xTaskCreatePinnedToCore(core_1_task, // Task function.
      "Task2", // name of task.
      10000, // Stack size of task
      NULL, // parameter of the task
      1, // priority of the task
      &task_2, // Task handle to keep track of created task
      1 // pin task to core 1
  ); 

  // Serial.print("[D] task_1, stack not used: ");
  // Serial.println(uxTaskGetStackHighWaterMark(task_1));

  // Serial.print("[D] task_2, stack not used: ");
  // Serial.println(uxTaskGetStackHighWaterMark(task_2));
}

void loop()
{
  vTaskDelete(NULL);

  // Memory Use Debug
  // unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis > printInterval) {
  //   previousMillis = currentMillis; 
  //   Serial.printf("[D] Free app space: %u\n", ESP.getFreeSketchSpace());
  //   Serial.printf("[D] Free Psram: %u\n", ESP.getFreePsram()); 
  //   Serial.printf("[D] Free Heap: %u\n", ESP.getFreeHeap());
  // }
  
}
