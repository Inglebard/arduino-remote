#include <WiFi.h>
#include <ArduinoOTA.h>
#include <IRremote.hpp>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"



// Replace with your network credentials
const char *ssid = "your SSID";
const char *password = "your password";
const char *espHostname = "Remote";
const char *ota_password = "your password";

AsyncWebServer server(80);

                     
#define RECV_PIN 32
#define SEND_PIN 4
#define TONE_LEDC_CHANNEL        1  // Using channel 1 makes tone() independent of receiving timer -> No need to stop receiving timer.
void tone(uint8_t _pin, unsigned int frequency){
    ledcAttachPin(_pin, TONE_LEDC_CHANNEL);
    ledcWriteTone(TONE_LEDC_CHANNEL, frequency);
}
void tone(uint8_t _pin, unsigned int frequency, unsigned long duration){
    ledcAttachPin(_pin, TONE_LEDC_CHANNEL);
    ledcWriteTone(TONE_LEDC_CHANNEL, frequency);
    delay(duration);
    ledcWriteTone(TONE_LEDC_CHANNEL, 0);
}
void noTone(uint8_t _pin){
    ledcWriteTone(TONE_LEDC_CHANNEL, 0);
}


void setOta()
{
    // OTA
  ArduinoOTA.setHostname(espHostname);
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
}

String processor(const String& var){
  return String();
}

void setup()
{
 
  Serial.begin(115200);

  WiFi.setHostname(espHostname);
  WiFi.mode(WIFI_STA);

  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  setOta();


  
  IrReceiver.begin(RECV_PIN);
  IrSender.begin(SEND_PIN,false);
  

  // static files
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  
  // main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("Request : /");
    request->send(SPIFFS, "/index.html", String(), false, processor); 
  });

  // AJAX change specific led color
  server.on("/send/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("Request : /send/");
    AsyncWebParameter* p_protocol = nullptr;      
    AsyncWebParameter* p_address = nullptr;         
    AsyncWebParameter* p_command = nullptr;    
    AsyncWebParameter* p_repeat = nullptr;   
      
    
    if(request->hasParam("protocol"))
      p_protocol = request->getParam("protocol");
      
    if(request->hasParam("address"))
      p_address = request->getParam("address");
      
    if(request->hasParam("command"))
      p_command = request->getParam("command");
      
    if(request->hasParam("repeat"))
      p_repeat = request->getParam("repeat");
      
    String protocol=p_protocol->value();
    String address=p_address->value();
    String command=p_command->value();
    uint8_t repeat=p_repeat->value().toInt();
    

    uint16_t i_address = strtol(address.c_str(),NULL,16);
    uint8_t i_command = strtol(command.c_str(),NULL,16);

    
    Serial.println(String(i_address));
    Serial.println(String(i_command));
    if(protocol == "NEC")
    {
      IrSender.sendNEC(i_address, i_command, true, repeat );      
    }
    request->send(200, "text/json", "OK"); 
  });
  
  
  server.begin();
  Serial.print("All start");
    
}


void loop()
{
  ArduinoOTA.handle();
  /*
  if (IrReceiver.decode())// Returns 0 if no data ready, 1 if data ready.     
  {      
   Serial.println(" ");     
   Serial.print("Code: ");
   IrReceiver.printIRResultShort(&Serial);
   Serial.println(" ");     
   IrReceiver.resume();
  }    
  /*
  uint16_t sAddress = 0x0;
  uint8_t sCommand = 0x11;
  uint8_t sRepeats = 0;

  
  Serial.println("send ");     
  IrSender.sendNEC(sAddress, sCommand, true,0 );
  //IrSender.sendRC6(0x0,0xD, true,0 );
  delay(2000);
  */
  
}
