/*
To remember: always save modifications!
Todo: 
- add local webpage to cycle between modes
- show future/previous days
- show temparature/cloudcoverage
- show sunrise/sunset
- discomode
- animate on phone notification
- change refresh rate
*/
#include "WifiLib.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

FASTLED_USING_NAMESPACE

// fastled setup

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    5
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    144
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          80
int LOWBRIGHTNESS = BRIGHTNESS - 45;
int HIGHBRIGHTNESS = BRIGHTNESS + 80;
#define FRAMES_PER_SECOND  60

// Wifi setup

WifiLib tel(true);

const char* ssid = tel.getSsid();
const char* password = tel.getPass();

bool showUpdates = true;


void PrintLn (String text){
  if (showUpdates){
    Serial.println(text); 
    }
}
void Print (String text){
  if (showUpdates){
    Serial.print(text); 
    }
}

void getLedSequence() {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://europe-west6-infra-data-280908.cloudfunctions.net/RetrieveMainCalendar");  //Specify request destination
    
    PrintLn("reaching site");
    int httpCode = http.GET();                                                                  //Send the request
    if (httpCode > 0) { //Check the returning code
      PrintLn(String(httpCode));
      String payload = http.getString(); //Get the request response payload
      PrintLn(payload);  
      
      PrintLn("payload above");//Print the response payload
      getJson(payload);
      
    }
    else{
      PrintLn("failed");
      String payload = http.getString();   //Get the request response payload
      PrintLn(payload);  
      }
    http.end();   //Close connection
    
  }
}

uint8_t Ary[432];
uint8_t i = 0, j = 0;


#define ARRAY_SIZE(array) ((sizeof(array))/(sizeof(array[0])))


void getJson (String textIn){
  DynamicJsonDocument doc(9999);

  // You can use a String as your JSON input.
  // WARNING: the string in the input  will be duplicated in the JsonDocument.
  deserializeJson(doc, textIn);
  JsonObject obj = doc.as<JsonObject>();
  lightString(obj);
  }

void lightString (JsonObject serie){
  for (int i = 0; i < 144; i++) {
    int R = serie[String("LedSequence")][i][0];
    int G = serie[String("LedSequence")][i][1];
    int B = serie[String("LedSequence")][i][2];
    leds[i] = CRGB( R, G, B);
    Print(String(R));
    Print(String(G));
    PrintLn(String(B));
    FastLED.show();
    delay(30);
  }
  
}

void setup () {
  // setup leds
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.clear();
  leds[0] = CRGB( 50, 100, 150);
  FastLED.show(); 

  //setup rest
  Serial.begin(115200);
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    leds[0] = CRGB( 20, 20, 20);
    FastLED.show(); 
    Print(".");
    delay(1000);
    leds[0] = CRGB( 50, 100, 150);
    FastLED.show(); 
  }
  PrintLn("");
  PrintLn("Connected");
  leds[0] = CRGB( 0, 50, 0);
  FastLED.show();
  getLedSequence();
}
 
void loop() {
  delay(60000);    //Send a request every 60 seconds
  getLedSequence();
}