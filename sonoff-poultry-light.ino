#include <ESP8266TimeAlarms.h>
#include "WifiConfig.h"
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
#include <aREST.h>
#include <aREST_UI.h>

aREST_UI rest = aREST_UI();

#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H


// Variables to be exposed to the API
float temperature;
float humidity;

#define ONBOARDLED 13 // Built in LED on SonOff th10/16
AlarmId id;


void setup()
{
  Serial.begin(115200);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);

  pinMode(ONBOARDLED, OUTPUT); // Onboard LED
  digitalWrite(ONBOARDLED, HIGH); // Switch off LED

  if ( MDNS.begin ( "plight" ) ) {
    Serial.println ( "MDNS responder started" );
  }
 // Set the title
  rest.title("Poultry Light");
  rest.button(13);

  temperature = 23.8;
  humidity = 39.1;
  
  rest.variable("temperature", &temperature);
  rest.variable("humidity", &humidity);
  rest.label("temperature");
  rest.label("humidity");
  rest.set_id("1");
  rest.set_name("esp8266");
  
  configTime(0, 0, "192.168.1.1","192.168.1.219", "0.se.pool.ntp.org");
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
  tzset();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

}

void loop()
{
    // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);
  
}


void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  time_t tnow = time(nullptr);
  
  snprintf ( temp, 400,

             "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Poultry Light ESP</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from the poultry!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Time: %s </p>\
    <p>Heapsize: %02d</p>\
  </body>\
</html>",

             hr, min % 60, sec % 60 ,ctime(&tnow)  , ESP.getFreeHeap()
           );
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );

}


// Check for Update
void checkhttpUpdate() {
  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.219", 80, "/ardup/arduino.php");

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      // Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s \n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());

      break;

    case HTTP_UPDATE_NO_UPDATES:
      //   Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }

}


