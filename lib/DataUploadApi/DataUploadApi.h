#ifndef DATA_UPLOAD_API_H
#define DATA_UPLOAD_API_H

#include <CellularNetwork800L.h>
#include "config.h"
#include <OAuth2.h>

class DataUploadApi: public OAuth2
{
public:
    CellularNetwork800L modem;
    TinyGsmClientSecure https_client;

    DataUploadApi(CellularNetwork800L &network, TinyGsmClientSecure &client, const string &host, const string &auth_path);
    ~DataUploadApi();

    void connectServer();

private:
    bool makeGSMConnection();
};

#endif