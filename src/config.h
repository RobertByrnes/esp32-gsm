#include <string>


using namespace std;


// OAuth Token
const string OAUTH_HOST = "draperbiotech.clystnet.com";
const string OAUTH_TOKEN_PATH = "/oauth/token";
const string GRANT_TYPE = "client_credentials";
const uint CLIENT_ID = 6;
const string CLIENT_SECRET = "RN6aJMOXOOSKcNBND2YOKuUQMKlKATgNu3iSYoHn";

// SIM card PIN, leave empty if not defined
const char *SIM_PIN = ""; 

// End point details for the server
const char SERVER[] = "draperbiotech.clystnet.com"; // Base url
const char AUTH_PATH[] = "/oauth/token"; // oAuth token path
const char RESOURCE_PATH[] = "/api/readings"; // Resource path
const uint16_t PORT = 443; // Server port number

// location of firmware file on external web server
// change to your actual .bin location
const char *UPDATE_HOST = "draperbiotech.clystnet.com";
const char *UPDATE_URL = "https://draperbiotech.clystnet.com/firmware.bin"; // must include either http:// or https://
const char *UPDATE_VERSION_FILE_URL = "https://draperbiotech.clystnet.com/firmware.txt";

// End point details for the API
const char DEVICE_ID[] = "";

// GPRS credentials
const char *APN = "giffgaff.com"; // APN Access Point Name
const char *GPRS_USER = "gg"; // GPRS User
const char *GPRS_PASSWORD = "p"; // GPRS Password

// WiFi credentials
const char *SSID = "CrowdedHouse"; // WiFi Network Name
const char *PASSWORD = "kF4QMhzc3xcS"; // WiFi Network Password

// Set the server connection interval
// #define CONNECTION_INTERVAL 3600000U // every hour
// #define CONNECTION_INTERVAL 3000000U // every 50mins
// #define CONNECTION_INTERVAL 2400000U // every 40mins
// #define CONNECTION_INTERVAL 1800000U // every 30mins
// #define CONNECTION_INTERVAL 1200000U // every 20mins
// #define CONNECTION_INTERVAL 900000U // every 15mins
// #define CONNECTION_INTERVAL 600000U // every 10mins
// #define CONNECTION_INTERVAL 300000U // every 5mins
// #define CONNECTION_INTERVAL 180000U // every 3mins
// #define CONNECTION_INTERVAL 120000U // every 2mins
#define CONNECTION_INTERVAL 60000U // every 1min

// ESP restart counter
unsigned int RESTART_COUNTER = 0;

// Global variables
int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware
unsigned long PREVIOUS_MILLIS = 0;