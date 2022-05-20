// This uncommented, enables debugging
// output via the serial monitor
#define DEV true

#ifdef DEV
// Aliases Serial interface for writing to the debugging
// console, with a default baud rate of 115200
    #define Debugger Serial
#endif

// GPRS credentials
const char apn[]      = "giffgaff.com"; // APN Access Poing Name
const char gprsUser[] = "gg"; // GPRS User
const char gprsPass[] = "p"; // GPRS Password

// SIM card PIN, leave empty if not defined
const char simPIN[]   = ""; 

// End point details for the server
const char server[] = "draperbiotech.clystnet.com"; // Base url
const char authPath[] = "/oauth/token"; // oAuth token path
const char resourcePath[] = "/api/readings"; // Resource path
const int port = 443; // Server port number

// End point details for the API
const char udid[] = "Proto2";
const char clientSecret[] = "RN6aJMOXOOSKcNBND2YOKuUQMKlKATgNu3iSYoHn";
String grantType = "client_credentials";
const int clientID = 6;

// SIM800 pins
#define MODEM_RST 5 // Reset pin
#define MODEM_PWKEY 4 // Enable pin
#define MODEM_POWER_ON 23 // Power pin
#define MODEM_TX 27 // Transmit pin
#define MODEM_RX 26 // Receive pin
#define I2C_SDA 21 // Serial data
#define I2C_SCL 22 // Serial clock

// Aliases Serial1 interface for AT 
// commands to SIM800 module
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800 // Modem is SIM800
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

// Registry config values for I2C
// with the SIM800 modem
#define IP5306_ADDR 0x75 // Initiate communication
#define IP5306_REG_SYS_CTL0 0x00 // Begin write

#define CONNECTION_INTERVAL 900000
#define CONNECTION_INTERVAL 3000
// ESP restart counter
unsigned int restart = 0;

// Counters
unsigned long augerPreviousMillis = 0;
unsigned long commsPreviousMillis = 0;
unsigned long faultPreviousMillis = 0;
const unsigned int augerPollingInterval = 700;
const unsigned int faultPollingInterval = 30000; 

// location of firmware file on external web server
// change to your actual .bin location
#define HOST "http://www.globalrbdev.uk/bin/main.ino.bin"

// Your WiFi credentials
const char* ssid = "CrowdedHouse";
const char* password = "kF4QMhzc3xcS";
// Global variables
int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware