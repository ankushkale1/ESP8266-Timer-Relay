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
#include "restore_factory_settings.h"
#include "Structs.h"

#define timeZone 5.5

#define TenSecs 10000000
#define OneMin 60000000
#define TenMins 600000000
#define TIMER_INTERVAL_MS 1000

ESP8266Timer ITimer;

int timer;
int alarmTriggerTime;
struct CurrentTime Current;
bool AlarmActive;
struct TimerDataStruct TimerData;
int WiFiTimer;
char factory_settings_stored[3];
bool OneSecoundPassed;
int timerArray[1440][4] = {};
bool WifiEnabled;
bool lastRelayStatus;
bool activeHigh;
int relayArray[1440] = {};
int relayPins[4] = {0, 2, 4, 5};

void ICACHE_RAM_ATTR TimerHandler(void)
{
  OneSecoundPassed = true;
}

void setup()
{

  Serial.begin(115200);
  Serial.println("Booting");
  EEPROM.begin(512);
  EEPROM_readAnything(150, factory_settings_stored);
  if (memcmp(&factory_settings_stored, "YES", 3) != 0)
  {
    restore_factory_settings();
  }
  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  for (int j = 0; j < 4; j++)
  {
    pinMode(relayPins[j], OUTPUT); // Initialise the output for the relay
  }

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
    EEPROM_readAnything(100, TimerData);
    setup_time(timeZone);
    int TimerOn[4];
    int TimerOff[4];
    for (int i = 0; i < 4; i++)
    {
      if (TimerData.TimerOn[i])
      {
        TimerOn[i] = TimerData.HourOn[i] * 60 + TimerData.MinuteOn[i];
        TimerOff[i] = TimerData.HourOff[i] * 60 + TimerData.MinuteOff[i];
        Serial.print(" >> Relay PIN: ");
        Serial.print(relayPins[i]);
        Serial.print(" >> TimerOn ");
        Serial.print(TimerOn[i]);
        Serial.print(" >> Timer OFF ");
        Serial.println(TimerOff[i]);
      }
      else
      {
        TimerOn[i] = 0;
        TimerOff[i] = 0;

        Serial.print(" >> Relay PIN: ");
        Serial.print(relayPins[i]);
        Serial.print(" >> TimerOn: 0 ");
        Serial.println(" >> Timer OFF: 0 ");
      }
    }
    for (int i = 0; i <= 1440; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        if (i >= TimerOn[j] && i < TimerOff[j])
        {
          timerArray[i][j] = 1;
          Serial.print(" >> Relay PIN: ");
          Serial.print(relayPins[i]);
          Serial.print(" >> Timer Array ");
          Serial.print(i);
          Serial.print(" ");
          Serial.print(j);
          Serial.print(" ");
          Serial.println(timerArray[i][j]);
        }
      }
    }
  }
  else
  {
    //    for(int i=2; i<=22; i+=4)
    //    {
    //      for(int j=(i*60); j<=(i+2)*60; j++)
    //      {
    //        timerArray[j][0] = 1;
    //        timerArray[j][1] = 1;
    //        timerArray[j][2] = 1;
    //        timerArray[j][3] = 1;
    //      }
    //    }

    Serial.println("Failed to enable WIFI");
  }

  Current = Current_Time();

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

  activeHigh = false;
}

void loop()
{

  if (WifiEnabled)
  {
    ArduinoOTA.handle();
    handle_client(Current, lastRelayStatus);

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
      Current = Current_Time();
      timer = micros();
    }
  }

  if (OneSecoundPassed)
  {
    updateLocalTime();
    OneSecoundPassed = false;
    int tempTime = (Current.Hour * 60) + Current.Minute;

    for (int j = 0; j < 4; j++)
    {
      if (timerArray[tempTime][j] == 1)
      {
        digitalWrite(relayPins[j], activeHigh ? HIGH : LOW); // Turn the relay on
        Serial.print(" >> RELAY: ");
        Serial.print(relayPins[j]);
        Serial.print(" ON. ");
        lastRelayStatus = true;
      }
      else
      {
        digitalWrite(relayPins[j], activeHigh ? LOW : HIGH); // Turn the relay off
        Serial.print(" >> RELAY: ");
        Serial.print(relayPins[j]);
        Serial.print(" OFF. ");
        lastRelayStatus = false;
      }
    }
  }
}

void updateLocalTime()
{
  Current.Second++;
  if (Current.Second >= 60)
  {
    Current.Second = 0;
    Current.Minute++;
    if (Current.Minute >= 60)
    {
      Current.Minute = 0;
      Current.Hour++;
      if (Current.Hour >= 24)
      {
        Current.Hour = 0;
        Current.Day++;
        if (Current.Day >= 7)
        {
          Current.Day = 0;
        }
      }
    }
  }
  char tempTime[6];
  if (Current.Minute < 10 && Current.Second < 10)
  {
    sprintf(tempTime, "0%d:0%d", Current.Minute, Current.Second);
  }
  else if (Current.Minute < 10)
  {
    sprintf(tempTime, "0%d:%d", Current.Minute, Current.Second);
  }
  else if (Current.Second < 10)
  {
    sprintf(tempTime, "%d:0%d", Current.Minute, Current.Second);
  }
  else
  {
    sprintf(tempTime, "%d:%d", Current.Minute, Current.Second);
  }
  Serial.print(Current.Hour);
  Serial.print(":");
  Serial.println(tempTime);
}
