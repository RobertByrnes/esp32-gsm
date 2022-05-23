#include "OAuth2.h"
#include <sstream>
#include <string.h>


// Constructor
OAuth2::OAuth2(const string host, const string auth_path): host(host), auth_path(auth_path), clientId(0) {};

// Destructor
OAuth2::~OAuth2() {};

/**
 * @brief Helper for converting uint data type to a string.
 * 
 * @return std::string
 */
std::string OAuth2::to_string(uint n) {
    std::ostringstream s;
    s << n;
    return s.str();
}

/**
 * @brief Set the credentials to be used for getting tokens.
 * 
 * @param grantType
 * @param clientId
 * @param clientSecret
 * 
 * @return bool
 */
bool OAuth2::setGrantType(const string grantType, uint clientId, const string clientSecret)
{
  this->grantType = grantType;
  this->clientId = clientId;
  this->clientSecret = clientSecret;
}

/**
 * @brief returns a string of the request body for requesting an auth token from the server.
 *
 * @return std::string
 */
std::string OAuth2::tokenRequestBody()
{
  string requestBody = "grant_type=" + this->grantType + "&client_id=" + this->to_string(this->clientId) + "&client_secret=" + this->clientSecret;
  return requestBody;
}

/**
 * @brief Extract JSON from server response
 * 
 * @param response 
 * @return String 
 */
std::string OAuth2::findJson(String response)
{
  Serial.println(response);
  int firstCurly = response.indexOf("{") + 1;
  int secondCurly = response.indexOf("}");
  // String JSON = "";
  String JSON = response.substring(firstCurly, secondCurly);
  
  // Serial.println(JSON);
  if (JSON.length() > 1)
    return JSON;
  else
    return String("");
}

/**
 * @brief Extract auth token from JSON string
 * 
 * @param JSON 
 * @return String 
 */
std::string OAuth2::extractToken(String JSON)
{
  int beginningOfToken = JSON.lastIndexOf(":") + 2;
  int endOfToken = JSON.lastIndexOf("\"");
  String accesToken = JSON.substring(beginningOfToken, endOfToken);
  Serial.println("[+] Acces Token: " + accesToken);

  if (accesToken.length() > 1)
    return accesToken;
  else
    return String("");
}

std::string OAuth2::personalAccessClientTokenRequestString()
{
  std::string requestBody = this->tokenRequestBody();
  std::string bodyLength = this->to_string(static_cast<uint>(requestBody.length()));
  std::string authRequest = "POST " + this->auth_path + " HTTP/1.1\r\n";
  authRequest.append("Host: " + this->host + "\r\n");
  authRequest.append("Accept: application/json\r\n");
  authRequest.append("Content-Type: application/x-www-form-urlencoded\r\n");
  authRequest.append("Content-Length: " + bodyLength + "\r\n\r\n");
  authRequest.append(requestBody);

  return authRequest;
}
