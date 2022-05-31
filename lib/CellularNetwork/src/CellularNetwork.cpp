#include "CellularNetwork.h"
#include <TinyGsmClient.h>

// Constructor
CellularNetwork::CellularNetwork(const char *apn, const char *gprs_user, const char *gprs_password, TinyGsm &modem):
  apn(apn), gprs_user(gprs_user), gprs_password(gprs_password), connection(modem) {}

// Destructor
CellularNetwork::~CellularNetwork() {}

/**
 * @brief Make up to 5 attempts to connect to the mobile network.
 * 
 * @param simPin const char *
 * 
 * @return bool
 */
bool CellularNetwork::initSim(const char *simPin) {
   // Set GSM module baud rate and UART pins
  SerialAT.begin(BAUD_RATE, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  this->connection.restart();

  // Unlock SIM card with a PIN if needed
  if (strlen(simPin) && this->connection.getSimStatus() != 3) {
    this->connection.simUnlock(simPin);
  }

  return true;
}

/**
 * @brief Make up to 5 attempts to connect to the mobile network.
 * 
 * @return bool
 */
bool CellularNetwork::connectNetwork()
{
  int i = 0;
  while (i <= 5) {
    ++i;
    if (!this->connection.gprsConnect(this->apn, this->gprs_user, this->gprs_password)) {
      delay(3000);
    } else {
      return true;
    }
  }
  return false;
}