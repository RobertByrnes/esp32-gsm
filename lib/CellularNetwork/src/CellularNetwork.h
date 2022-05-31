#ifndef CELLULAR_NETWORK_H
#define CELLULAR_NETWORK_H

#include <Arduino.h>
#include "esp32_config.h"
#include <HardwareSerial.h>
#include <TinyGsmClient.h>


class CellularNetwork
{

public:
    const char *apn;
    const char *gprs_user;
    const char *gprs_password;
    TinyGsm connection;

    CellularNetwork(const char *apn, const char *gprs_user, const char *gprs_password, TinyGsm &modem);

    ~CellularNetwork();

    bool initSim(const char *simPin);
    bool connectNetwork();
};

#endif