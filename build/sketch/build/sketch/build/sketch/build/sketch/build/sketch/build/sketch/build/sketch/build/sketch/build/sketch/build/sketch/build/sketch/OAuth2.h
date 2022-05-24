#ifndef OAUTH2_H
#define OAUTH2_H

#include <string>

using namespace std;


class OAuth2
{
public:
    const string host;
    const string auth_path;

    OAuth2(const string host, const string auth_path);

    ~OAuth2();

    bool setGrantType(const string grantType, uint clientID, const string clientSecret);
    string getToken(string httpResponse);
    string personalAccessClientTokenRequestString();

private:
    uint clientId;
    string clientSecret;
    string grantType;

    string extractToken(string JSON);
    string findJson(string response);
    string tokenRequestBody();
    string to_string(uint n);
};

#endif