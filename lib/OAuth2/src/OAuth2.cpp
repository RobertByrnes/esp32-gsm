#include "OAuth2.h"
#include <sstream>
#include <string.h>


// Constructor
OAuth2::OAuth2(const string& host, const string& auth_path): host(host), auth_path(auth_path), clientId(0) {};

// Destructor
OAuth2::~OAuth2() {};

/**
 * @brief Extract Auth token from JSON string.
 * 
 * @param JSON std::string
 * 
 * @return std::string 
 */
string OAuth2::extractToken(string JSON) // private
{
  int beginningOfToken = JSON.find_last_of(":") + 2;
  int endOfToken = JSON.find_last_of("\"}") - 1;
 
  string accesToken = JSON.substr(beginningOfToken, endOfToken - beginningOfToken);

  if (accesToken.length() > 1)
    return accesToken;
  else
    return string("");
}

/**
 * @brief Extract JSON from server response
 * 
 * @param response std::string 
 * 
 * @return std::string 
 */
string OAuth2::findJson(string response) // private
{
  int firstCurly = response.find_first_of("{") + 1;
  int secondCurly = response.find_first_of("}");
  string JSON = response.substr(firstCurly, secondCurly);
  
  if (JSON.length() > 1)
    return JSON;
  else
    return string("");
}

/**
 * @brief Extract Auth token from the HTTP response returned from the token endpoint.
 * 
 * @param httpResponse std::string
 * 
 * @return const char * (either the token or an empty string)
 */
const char * OAuth2::getToken(string httpResponse) // public
{
    string parsedResponse = this->findJson(httpResponse);

    if (parsedResponse != "") {
      return this->extractToken(parsedResponse).c_str();
    } else {
      string notFound = "";
      return notFound.c_str();
    }
}

/**
 * @brief get the request body for the request to the token endpoint.
 * 
 * @return const char *
 */
const char * OAuth2::personalAccessClientTokenRequestString() // public
{
  string requestBody = this->tokenRequestBody();
  string bodyLength = this->to_string(static_cast<uint>(requestBody.length()));
  string authRequest = "POST " + this->auth_path + " HTTP/1.1\r\n";
  authRequest.append("Host: " + this->host + "\r\n");
  authRequest.append("Accept: application/json\r\n");
  authRequest.append("Content-Type: application/x-www-form-urlencoded\r\n");
  authRequest.append("Content-Length: " + bodyLength + "\r\n\r\n");
  authRequest.append(requestBody);

  return authRequest.c_str();
}

/**
 * @brief Set the credentials to be used for getting tokens.
 * 
 * @param grantType std::string
 * @param clientId uint
 * @param clientSecret std::string
 * 
 * @return void
 */
void OAuth2::setGrantType(const string &grantType, const uint clientId, const string &clientSecret) // public
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
string OAuth2::tokenRequestBody() // private
{
  string requestBody = "grant_type=" + this->grantType + "&client_id=" + this->to_string(this->clientId) + "&client_secret=" + this->clientSecret;
  return requestBody;
}

/**
 * @brief Helper for converting uint data type to a string.
 * 
 * @param n uint
 * 
 * @return std::string
 */
string OAuth2::to_string(uint n) // private
{
    ostringstream s;
    s << n;
    return s.str();
}
