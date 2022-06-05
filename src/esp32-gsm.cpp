#include <Arduino.h>
#include <DataUploadApi.h>
#include "WiFi_FirmwareUpdater.h"
#include "config.h"
#include <ESP32TimerInterrupt.h>
#include <SimpleTimer.h>   

// Timer demonstration
#define LED_PIN 13
volatile int interruptCounter_1; // volatile as being shared between ISR and main
volatile int interruptCounter_2; // volatile as being shared between ISR and main
int totalInterruptCounter;

// ESP hardware timers
hw_timer_t *timer_1 = NULL; 
hw_timer_s *timer_2 = NULL; 
portMUX_TYPE timerMux_1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux_2 = portMUX_INITIALIZER_UNLOCKED;

// Multicore tasking
TaskHandle_t task_1;
TaskHandle_t task_2;

// Modem
TinyGsm modem(SerialAT);
CellularNetwork800L network(APN, GPRS_USER, GPRS_PASSWORD, modem);

// HTTP(S) Client
TinyGsmClientSecure client(modem);

// Authentication
DataUploadApi api(network, client, OAUTH_HOST, OAUTH_TOKEN_PATH);

// Updates
WiFi_FirmwareUpdater update(SSID, PASSWORD, CURRENT_VERSION);


/**
 * @brief Interrupt service routine callback func,
 * increment counter shared between IRAM and main.
 * Runs on core 0.
 * 
 * @return void
 */
void IRAM_ATTR core0_ISR()
{
  portENTER_CRITICAL_ISR(&timerMux_1);
  interruptCounter_1++;
  portEXIT_CRITICAL_ISR(&timerMux_1);
}

/**
 * @brief Interrupt service routine callback func,
 * increment counter shared between IRAM and main.
 * Runs on core 1 with main application.
 * 
 * @return void
 */
void IRAM_ATTR core1_ISR()
{
  portENTER_CRITICAL_ISR(&timerMux_2);
  interruptCounter_2++;
  portEXIT_CRITICAL_ISR(&timerMux_2);
}


static void core0_task(void *pvParameters)
{
  Serial.print( "[i] Task_1 running on: core " );
  Serial.println( xPortGetCoreID() );

  // ISR 1
  timer_1 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer_1, &core0_ISR, true); // boolean is for edge
  timerAlarmWrite(timer_1, 10000000, true); // 10sec
  timerAlarmEnable(timer_1);

  for (;;) {
    if (interruptCounter_1 > 0) {
      portENTER_CRITICAL(&timerMux_1);
      interruptCounter_1--;
      portEXIT_CRITICAL(&timerMux_1);

      totalInterruptCounter++;

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

static void core1_task(void *pvParameters)
{
  Serial.print( "[i] Task_2 running on: core " );
  Serial.println( xPortGetCoreID() );

  // ISR 2
  timer_2 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer_2, &core1_ISR, true); // boolean is for edge
  timerAlarmWrite(timer_2, 60000000, true); // 10sec
  timerAlarmEnable(timer_2);

  for (;;) {
    if (interruptCounter_2 > 0) {
      portENTER_CRITICAL(&timerMux_2);
      interruptCounter_2--;
      portEXIT_CRITICAL(&timerMux_2);

      totalInterruptCounter++;

      api.connectServer(APN, SERVER, PORT);
    }
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);

  pinMode(LED_PIN, OUTPUT);

  // Disable core watchdogs
  disableCore0WDT();
  disableCore1WDT();

  xTaskCreatePinnedToCore(core0_task, // Task function.
      "Task1", // name of task.
      10000, // Stack size of task
      NULL, // parameter of the task
      1, // priority of the task
      &task_1, // Task handle to keep track of created task
      0 // pin task to core 0
  ); 

  sleep(2);

  xTaskCreatePinnedToCore(core1_task, // Task function.
      "Task2", // name of task.
      10000, // Stack size of task
      NULL, // parameter of the task
      1, // priority of the task
      &task_2, // Task handle to keep track of created task
      1 // pin task to core 1
  ); 
}

void loop() { vTaskDelete(NULL); }
