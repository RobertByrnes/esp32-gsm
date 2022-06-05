#ifndef CONFIG_H
#define CONFIG_H

#include <string>
using namespace std;

// Version
const string CURRENT_VERSION = "0.0.1"; 

// Hardware ISR timer setup
#define _TIMERINTERRUPT_LOGLEVEL_ 4
#define HW_TIMER_INTERVAL_US 10000L
volatile uint32_t startMillis = 0;

// Individual timer config
#define GSM_INTERVAL_MS 60000U
#define WIFI_INTERVAL_MS 90000U

// Set the timer interval
// #define CONNECTION_INTERVAL 3600000L // every hour
// #define CONNECTION_INTERVAL 3000000L // every 50mins
// #define CONNECTION_INTERVAL 2400000L // every 40mins
// #define CONNECTION_INTERVAL 1800000L // every 30mins
// #define CONNECTION_INTERVAL 1200000L // every 20mins
// #define CONNECTION_INTERVAL 900000L // every 15mins
// #define CONNECTION_INTERVAL 600000L // every 10mins
// #define CONNECTION_INTERVAL 300000L // every 5mins
// #define CONNECTION_INTERVAL 180000L // every 3mins
// #define CONNECTION_INTERVAL 120000L // every 2mins
// #define CONNECTION_INTERVAL 60000L // every 1min

// GPRS credentials
const char *APN = "giffgaff.com"; // APN Access Point Name
const char *GPRS_USER = "gg"; // GPRS User
const char *GPRS_PASSWORD = "p"; // GPRS Password

// End point details for the server
const char SERVER[] = "draperbiotech.clystnet.com"; // Base url
const char AUTH_PATH[] = "/oauth/token"; // oAuth token path
const char RESOURCE_PATH[] = "/api/readings"; // Resource path
const uint16_t PORT = 443; // Server port number

// OAuth Token
const string OAUTH_HOST = "draperbiotech.clystnet.com";
const string OAUTH_TOKEN_PATH = "/oauth/token";

// location of firmware file on external web server
// change to your actual .bin location
const char *UPDATE_HOST = "draperbiotech.clystnet.com";
const char *UPDATE_URL = "https://draperbiotech.clystnet.com/firmware.bin"; // must include either http:// or https://
const char *UPDATE_VERSION_FILE_URL = "https://draperbiotech.clystnet.com/firmware.txt";

// End point details for the API
const char DEVICE_ID[] = "";

// WiFi credentials
const char *SSID = "CrowdedHouse"; // WiFi Network Name
const char *PASSWORD = "kF4QMhzc3xcS"; // WiFi Network Password

// ESP restart counter
unsigned int RESTART_COUNTER = 0;

// Counters
unsigned long PREVIOUS_MILLIS = 0;

#endif