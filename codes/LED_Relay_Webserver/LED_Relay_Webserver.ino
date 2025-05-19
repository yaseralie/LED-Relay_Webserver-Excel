//Display text in LED matrix Max7219, sent from webserver
//Dont forget to install or add zip library ESPAsyncWebServer:https://github.com/me-no-dev/ESPAsyncWebServer
//LED matrix library MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//Reference GPIO  https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "SSID";
const char* password = "Password";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";
const char* PARAM_INPUT_4 = "input4";
const char* PARAM_INPUT_5 = "input5";

//Setup for LED Max7219==============================
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN 15
// Hardware SPI connection
MD_Parola LED = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
//====================================================

//configuration=======================================
String txt_default = "Keep a safe distance!";
String msg = txt_default;
bool relay;
int relay_no;
String status_relay;
//===================================================

//Variables for output relay===============================
int relay1 = 5; //button D0
int relay2 = 4; //button D1
int relay3 = 0; //button D2
int relay4 = 2; //button D3
//===================================================

// HTML web page to handle 2 input fields (input1, input2)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
<title>ESP-01 RELAY Control</title>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;
text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
.button2 {background-color: #77878A;}</style>
</head>
<body>
<h1 align = center>ESP8266 LED Matrix & Relay Control</h1>
  <form action="/get">
    Send Customize Text: <input type="text" name="input1">
    <input type="submit" value="Submit" class="button">
  </form><br>
<a href="/get?input2=ON"><button class="button">Relay #1 TURN ON</button></a>
<a href="/get?input2=OFF"><button class="button button2">Relay #1 TURN OFF</button></a><br>
<a href="/get?input3=ON"><button class="button">Relay #2 TURN ON</button></a>
<a href="/get?input3=OFF"><button class="button button2">Relay #2 TURN OFF</button></a><br>
<a href="/get?input4=ON"><button class="button">Relay #3 TURN ON</button></a>
<a href="/get?input4=OFF"><button class="button button2">Relay #3 TURN OFF</button></a><br>
<a href="/get?input5=ON"><button class="button">Relay #4 TURN ON</button></a>
<a href="/get?input5=OFF"><button class="button button2">Relay #4 TURN OFF</button></a><br>
</body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  //Start LED
  LED.begin();

  //setup button output===============
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  //turn off all relay using high
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  //=================================

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      relay = false;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      status_relay = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      relay = true;
      relay_no = 1;
    }
        // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      status_relay = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      relay = true;
      relay_no = 2;
    }
        // GET input4 value on <ESP_IP>/get?input4=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_4)) {
      status_relay = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      relay = true;
      relay_no = 3;
    }
        // GET input5 value on <ESP_IP>/get?input5=<inputMessage>
  else if (request->hasParam(PARAM_INPUT_5)) {
      status_relay = request->getParam(PARAM_INPUT_5)->value();
      inputParam = PARAM_INPUT_5;
      relay = true;
      relay_no = 4;
    }
    else {
      inputMessage = "";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    //Display Text
    if (inputMessage != "")
    {
      msg = inputMessage;
    }

    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                  + inputParam + ") with value: " + inputMessage +
                  "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  //control relay=======================================
  if (relay == true)
  {
    if (relay_no == 1)
    {
      if (status_relay == "ON")
      {
        digitalWrite(relay1, LOW);
      }
      else
      {
        digitalWrite(relay1, HIGH);
      }
    }
        if (relay_no == 2)
    {
      if (status_relay == "ON")
      {
        digitalWrite(relay2, LOW);
      }
      else
      {
        digitalWrite(relay2, HIGH);
      }
    }
        if (relay_no == 3)
    {
      if (status_relay == "ON")
      {
        digitalWrite(relay3, LOW);
      }
      else
      {
        digitalWrite(relay3, HIGH);
      }
    }
        if (relay_no == 4)
    {
      if (status_relay == "ON")
      {
        digitalWrite(relay4, LOW);
      }
      else
      {
        digitalWrite(relay4, HIGH);
      }
    }
  }
  //===================================================
    //show LED matrix====================================
    if (LED.displayAnimate())
      LED.displayText(msg.c_str(), PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    //===================================================


}
