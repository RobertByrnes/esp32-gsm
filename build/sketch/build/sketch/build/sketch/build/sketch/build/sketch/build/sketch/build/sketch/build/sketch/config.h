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
const int PORT = 443; // Server port number

// End point details for the API
const char DEVICE_ID[] = "";

// GPRS credentials
const char *APN = "giffgaff.com"; // APN Access Point Name
const char *GPRS_USER = "gg"; // GPRS User
const char *GPRS_PASSWORD = "p"; // GPRS Password

// WiFi credentials
const char *SSID = "CrowdedHouse"; // WiFi Network Name
const char *PASSWORD = "kF4QMhzc3xcS"; // WiFi Network Password

// Set the server polling time
// #define CONNECTION_INTERVAL 900000 // every 15mins
// #define CONNECTION_INTERVAL 300000 // every 5mins
#define CONNECTION_INTERVAL 60000 // every 1min

// ESP restart counter
unsigned int RESTART_COUNTER = 0;

// location of firmware file on external web server
// change to your actual .bin location
#define HOST "http://www.globalrbdev.uk/bin/main.ino.bin"

// Global variables
int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware
unsigned long PREVIOUS_MILLIS = 0;