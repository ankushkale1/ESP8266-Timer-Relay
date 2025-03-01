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
#include <ESP8266mDNS.h>

#define TIMER_INTERVAL_MS 1000

ESP8266Timer ITimer;

int timer;
struct CurrentTime currentTime;
struct TimerDataStruct timerSettings;
int WiFiTimer;
char factory_settings_stored[3];
bool OneSecoundPassed;
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
  JsonArray secondOn = doc.createNestedArray("secondOn");
  JsonArray secondOff = doc.createNestedArray("secondOff");
  JsonArray timerOn = doc.createNestedArray("timerOn");
  JsonArray gpioPin = doc.createNestedArray("gpioPin");
  JsonArray activeHigh = doc.createNestedArray("activeHigh");

  for(int i=0;i<NO_OF_TIMERS;i++){
    hourOn.add(timerSettings.hourOn[i]);
    hourOff.add(timerSettings.hourOff[i]);
    minuteOn.add(timerSettings.minuteOn[i]);
    minuteOff.add(timerSettings.minuteOff[i]);
    secondOn.add(timerSettings.secondOn[i]);
    secondOff.add(timerSettings.secondOff[i]);
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
  EEPROM.begin(EEPROM_SIZE);
  EEPROM_readAnything(OK_LOCATION, factory_settings_stored);
  if (memcmp(&factory_settings_stored, "YES", 3) != 0)
  {
    restore_factory_settings();
    Serial.println("WARNING !!! Something is corrupt so restoring factory settings");
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
  }
  else
  {
    Serial.println("Lets Go");
    WifiEnabled = true;
  }

  if (WifiEnabled)
  {
    start_server();
    SetupOTA();
    EEPROM_readAnything(SETTINGS_LOCATION, timerSettings);
    setup_time(timerSettings.timezone);

    for (int j = 0; j < NO_OF_TIMERS; j++)
    {
      if (timerSettings.timerOn[j]){
        pinMode(timerSettings.gpioPin[j], OUTPUT); // Initialise the output for the relay
      }
    }

    Serial.println("=========== Reading Saved Data =============");
    printSettings();
    Serial.println("=========== Done : Reading Saved Data =============");
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

  if (!LittleFS.begin()) {
      Serial.println("Failed to mount LittleFS");
      return;
  }

  if (!MDNS.begin(timerSettings.hostname)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  goToSleep();  // Start in sleep mode
}

void loop()
{
  if (WifiEnabled)
  {
    ArduinoOTA.handle();
    handle_client(currentTime, lastRelayStatus);

    if ((micros() - WiFiTimer) > 600000000) // check if wifi connection lost and if so try to reconnect
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        ESP.restart(); // try to reconnect rather than resetting
      }
      WiFiTimer = micros();
    }

    // update current hour from NTP server
    if ((micros() - timer) > 600000000)
    {
      currentTime = Current_Time();
      timer = micros();
    }
  }

  if (OneSecoundPassed)
  {
    updateLocalTime();
    OneSecoundPassed = false;

    for (int j = 0; j < NO_OF_TIMERS; j++)
    {
      if (timerSettings.timerOn[j] && !timerSettings.allOff)
      {
        // Calculate total seconds for start and end times
        int startTime = timerSettings.hourOn[j] * 3600 + timerSettings.minuteOn[j] * 60 + timerSettings.secondOn[j];
        int endTime = timerSettings.hourOff[j] * 3600 + timerSettings.minuteOff[j] * 60 + timerSettings.secondOff[j];
        int currentTimeInSeconds = currentTime.Hour * 3600 + currentTime.Minute * 60 + currentTime.Second;

        // Handle midnight rollover (e.g., 23:59:59 to 00:00:00)
        if (endTime < startTime) {
          if (currentTimeInSeconds >= startTime || currentTimeInSeconds < endTime) {
            digitalWrite(timerSettings.gpioPin[j], timerSettings.activeHigh[j] ? HIGH : LOW); // Turn the relay on
            Serial.print(" >> RELAY: ");
            Serial.print(timerSettings.gpioPin[j]);
            Serial.print(" ON. ");
            lastRelayStatus = true;
          } else {
            digitalWrite(timerSettings.gpioPin[j], timerSettings.activeHigh[j] ? LOW : HIGH); // Turn the relay off
            Serial.print(" >> RELAY: ");
            Serial.print(timerSettings.gpioPin[j]);
            Serial.print(" OFF. ");
            lastRelayStatus = false;
          }
        } else {
          if (currentTimeInSeconds >= startTime && currentTimeInSeconds < endTime) {
            digitalWrite(timerSettings.gpioPin[j], timerSettings.activeHigh[j] ? HIGH : LOW); // Turn the relay on
            Serial.print(" >> RELAY: ");
            Serial.print(timerSettings.gpioPin[j]);
            Serial.print(" ON. ");
            lastRelayStatus = true;
          } else {
            digitalWrite(timerSettings.gpioPin[j], timerSettings.activeHigh[j] ? LOW : HIGH); // Turn the relay off
            Serial.print(" >> RELAY: ");
            Serial.print(timerSettings.gpioPin[j]);
            Serial.print(" OFF. ");
            lastRelayStatus = false;
          }
        }
      }
    }

    if (!getIsSleeping()) {
        Serial.println("🌙 Going back to sleep...");
        goToSleep(); // Put back to sleep after handling request
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
  char tempTime[9];
  sprintf(tempTime, "%02d:%02d:%02d", currentTime.Hour, currentTime.Minute, currentTime.Second);
  Serial.println(tempTime);
}