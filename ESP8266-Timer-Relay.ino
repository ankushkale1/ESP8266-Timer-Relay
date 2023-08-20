/* This code has the following features.
 *  If no wifi is set it will startup as an AP to which you can connect to via a web browser and using the gateway address. This can be used to configure the wifi SSID and password
 *  If you do not connect to the device while it is in AP mode it will timeout after 120 seconds and start a default program where it turns the relay on and off in 2 hour cycles.
 *  If you do connect and setup the device to connect to the wifi it will startup and poll an NTP server to get the current time. it will turn on and off the relay output with respect to a configured time based program
 *  The time based program can be configured via the web portal which will be accessible via the IP address of the device. Check your AP gateway for connected devices to find the IP address of the timer relay
 */

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "EEPROMAnything.h"
#include "get_time.h"
#include "OTA.h"
#include <stdio.h>
#include <stdint.h>
#include "web_portal.h"
#include "ESP8266TimerInterrupt.h"
#include "Structs.h"
#include <LittleFS.h>

//#define TenSecs 10000000
//#define OneMin 60000000
#define TenMins 600000000
#define TIMER_INTERVAL_MS 1000

ESP8266Timer ITimer;

int timer;
int alarmTriggerTime;
struct CurrentTime currentTime;
struct TimerDataStruct timerSettings;
int WiFiTimer;
char factory_settings_stored[3];
bool OneSecoundPassed;
//first index for which minut, 2nd for timer, so it says for which minut which timer is set
bool timerArray[1440][NO_OF_TIMERS] = {};
bool WifiEnabled;
bool lastRelayStatus;

void ICACHE_RAM_ATTR TimerHandler(void)
{
  OneSecoundPassed = true;
}

void printSettings(){
  DynamicJsonDocument doc(1536);
  JsonArray hourOn = doc.createNestedArray("hourOn");
  JsonArray hourOff = doc.createNestedArray("hourOff");
  JsonArray minuteOn = doc.createNestedArray("minuteOn");
  JsonArray minuteOff = doc.createNestedArray("minuteOff");
  JsonArray timerOn = doc.createNestedArray("timerOn");
  JsonArray gpioPin = doc.createNestedArray("gpioPin");
  JsonArray activeHigh = doc.createNestedArray("activeHigh");

  for(int i=0;i<NO_OF_TIMERS;i++){
    hourOn.add(timerSettings.hourOn[i]);
    hourOff.add(timerSettings.hourOff[i]);
    minuteOn.add(timerSettings.minuteOn[i]);
    minuteOff.add(timerSettings.minuteOff[i]);
    timerOn.add(timerSettings.timerOn[i]);
    gpioPin.add(timerSettings.gpioPin[i]);
    activeHigh.add(timerSettings.activeHigh[i]);
  }

  doc["allOff"] = timerSettings.allOff;
  doc["timezone"] = timerSettings.timezone;  

  String jsonData;
  serializeJson(doc, jsonData);

  Serial.println(jsonData);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");
  EEPROM.begin(512);
  EEPROM_readAnything(500, factory_settings_stored);
  if (memcmp(&factory_settings_stored, "YES", 3) != 0)
  {
    restore_factory_settings();
    Serial.println("Something is currupt so restoring factory settings");
  }
  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  wm.setConfigPortalTimeout(120); // auto close configportal after n seconds

  bool response;
  response = wm.autoConnect("AutoConnectAP"); // anonymous ap
  if (!response)
  {
    Serial.println("Failed to connect");
    WifiEnabled = false;
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("Lets Go");
    WifiEnabled = true;
  }

  if (WifiEnabled)
  {
    start_server();
    SetupOTA();
    EEPROM_readAnything(100, timerSettings);
    setup_time(timerSettings.timezone);

    for (int j = 0; j < NO_OF_TIMERS; j++)
    {
      if (timerSettings.timerOn[j]){
        pinMode(timerSettings.gpioPin[j], OUTPUT); // Initialise the output for the relay
      }
    }

    int timerOn[NO_OF_TIMERS];  //when to start in minuts
    int timerOff[NO_OF_TIMERS]; //when to stop in minuts

    Serial.println("=========== Reading Saved Data =============");
    printSettings();
    Serial.println("=========== Done : Reading Saved Data =============");

    for (int i = 0; i < NO_OF_TIMERS; i++)
    {
      if (timerSettings.timerOn[i])
      {
        timerOn[i] = timerSettings.hourOn[i] * 60 + timerSettings.minuteOn[i];
        timerOff[i] = timerSettings.hourOff[i] * 60 + timerSettings.minuteOff[i];
      }
      else
      {
        timerOn[i] = 0;
        timerOff[i] = 0;
      }
    }

    for (int i = 0; i <= 1440; i++) // 1440 minuts per day as we have 24 hours, each with 60 mins so 24*60 = 1440
    {
      for (int j = 0; j < NO_OF_TIMERS; j++)
      {
        if (i >= timerOn[j] && i < timerOff[j])
        {
          if(!timerSettings.allOff){
            timerArray[i][j] = true;
          }else{
            Serial.println("All Relay off set");
          }
        }
      }
    }
  }
  else
  {
    Serial.println("Failed to enable WIFI");
  }

  currentTime = Current_Time();

  timer = micros();
  WiFiTimer = timer;

  // Interval in microsecs
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    Serial.println("Starting  ITimer OK, millis() = " + String(millis()));
  }
  else
  {
    Serial.println("Can't set ITimer correctly. Select another freq. or interval");
  }

  // if (!SPIFFS.begin()) {
  //   Serial.println("Failed to mount file system");
  //   return;
  // }

  if (!LittleFS.begin()) {
      Serial.println("Failed to mount LittleFS");
      return;
  }
}

void loop()
{
  if (WifiEnabled)
  {
    ArduinoOTA.handle();
    handle_client(currentTime, lastRelayStatus);

    if ((micros() - WiFiTimer) > TenMins) // check if wifi connection lost and if so try to reconnect
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        ESP.restart(); // try to reconnect rather than resetting
      }
      WiFiTimer = micros();
    }

    // update current hour from NTP server
    if ((micros() - timer) > TenMins)
    {
      currentTime = Current_Time();
      timer = micros();
    }
  }

  if (OneSecoundPassed)
  {
    updateLocalTime();
    OneSecoundPassed = false;
    int tempTime = (currentTime.Hour * 60) + currentTime.Minute;

    for (int j = 0; j < NO_OF_TIMERS; j++)
    {
      //so for current minute does any timer set ?
      if (timerArray[tempTime][j])
      {
        digitalWrite(timerSettings.gpioPin[j], timerSettings.activeHigh[j] ? HIGH : LOW); // Turn the relay on
        Serial.print(" >> RELAY: ");
        Serial.print(timerSettings.gpioPin[j]);
        Serial.print(" ON. ");
        lastRelayStatus = true;
      }
      else
      {
        bool skipThisPin = false;
        for (int k = 0; k < NO_OF_TIMERS; k++)
        {
          if (timerArray[tempTime][k] && (timerSettings.gpioPin[k] == timerSettings.gpioPin[j])){
            skipThisPin = true;
            break;
          }
        }

        if(!skipThisPin){
          digitalWrite(timerSettings.gpioPin[j], timerSettings.activeHigh[j] ? LOW : HIGH); // Turn the relay off
          Serial.print(" >> RELAY: ");
          Serial.print(timerSettings.gpioPin[j]);
          Serial.print(" OFF. ");
        }
        else{
          Serial.print(" >> RELAY: ");
          Serial.print(timerSettings.gpioPin[j]);
          Serial.print(" Skipped. ");
        }
        lastRelayStatus = false;
      }
    }
  }
}

void updateLocalTime()
{
  currentTime.Second++;
  if (currentTime.Second >= 60)
  {
    currentTime.Second = 0;
    currentTime.Minute++;
    if (currentTime.Minute >= 60)
    {
      currentTime.Minute = 0;
      currentTime.Hour++;
      if (currentTime.Hour >= 24)
      {
        currentTime.Hour = 0;
        currentTime.Day++;
        if (currentTime.Day >= 7)
        {
          currentTime.Day = 0;
        }
      }
    }
  }
  char tempTime[6];
  if (currentTime.Minute < 10 && currentTime.Second < 10)
  {
    sprintf(tempTime, "0%d:0%d", currentTime.Minute, currentTime.Second);
  }
  else if (currentTime.Minute < 10)
  {
    sprintf(tempTime, "0%d:%d", currentTime.Minute, currentTime.Second);
  }
  else if (currentTime.Second < 10)
  {
    sprintf(tempTime, "%d:0%d", currentTime.Minute, currentTime.Second);
  }
  else
  {
    sprintf(tempTime, "%d:%d", currentTime.Minute, currentTime.Second);
  }
  Serial.print(currentTime.Hour);
  Serial.print(":");
  Serial.println(tempTime);
}
