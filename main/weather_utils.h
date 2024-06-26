// Weather includes and Global
#include <Arduino_JSON.h>
#include <HTTPClient.h>

String openWeatherMapApiKey = "a984709083050d06f192c2d24b5229ad";
unsigned long weather_timer = -60000;
String countryCode = "Haifa";
String city = "IL";
String jsonBuffer;
int currentTemperature = 0;
int weatherCondition = 0;


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    // Serial.print("HTTP Response code: ");
    // Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
