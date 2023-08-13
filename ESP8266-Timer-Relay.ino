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
#include "common.h"

//#define TenSecs 10000000
//#define OneMin 60000000
#define TenMins 600000000
#define TIMER_INTERVAL_MS 1000

ESP8266Timer ITimer;

int timer;
int alarmTriggerTime;
struct CurrentTime Current;
struct TimerDataStruct timerData;
int WiFiTimer;
char factory_settings_stored[3];
bool OneSecoundPassed;
//first index for which minut, 2nd for timer, so it says for which minut which timer is set
int timerArray[1440][NO_OF_TIMERS] = {};
bool WifiEnabled;
bool lastRelayStatus;

void ICACHE_RAM_ATTR TimerHandler(void)
{
  OneSecoundPassed = true;
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
    EEPROM_readAnything(100, timerData);
    setup_time(timeZone);

    for (int j = 0; j < NO_OF_TIMERS; j++)
    {
      pinMode(timerData.GPIOPin[j], OUTPUT); // Initialise the output for the relay
    }

    int TimerOn[NO_OF_TIMERS];  //when to start in minuts
    int TimerOff[NO_OF_TIMERS]; //when to stop in minuts

    Serial.println("=========== Reading Saved Data =============");
    for (int i = 0; i < NO_OF_TIMERS; i++)
    {
      if (timerData.TimerOn[i])
      {
        TimerOn[i] = timerData.HourOn[i] * 60 + timerData.MinuteOn[i];
        TimerOff[i] = timerData.HourOff[i] * 60 + timerData.MinuteOff[i];
        Serial.print(" >> Relay PIN: ");
        Serial.println(timerData.GPIOPin[i]);
        Serial.print(" >> Relay PinType Active High: ");
        Serial.println(timerData.activeHigh[i]);
        Serial.print(" >> TimerOn ");
        Serial.print(TimerOn[i]/60);
        Serial.print(":");
        Serial.println(TimerOn[i]%60);
        Serial.print(" >> Timer OFF ");
        Serial.print(TimerOff[i]/60);
        Serial.print(":");
        Serial.println(TimerOff[i]%60);
      }
      else
      {
        TimerOn[i] = 0;
        TimerOff[i] = 0;

        Serial.print(" >> Relay PIN: ");
        Serial.println(timerData.GPIOPin[i]);
        Serial.print(" >> Relay PinType Active High: ");
        Serial.println(timerData.activeHigh[i]);
        Serial.println(" >> TimerOn: 0 ");
        Serial.println(" >> Timer OFF: 0 ");
      }
    }
    Serial.println("=========== Done : Reading Saved Data =============");

    for (int i = 0; i <= 1440; i++) // 1440 minuts per day as we have 24 hours, each with 60 mins so 24*60 = 1440
    {
      for (int j = 0; j < NO_OF_TIMERS; j++)
      {
        if (i >= TimerOn[j] && i < TimerOff[j])
        {
          timerArray[i][j] = 1;
        }
      }
    }
  }
  else
  {
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

    for (int j = 0; j < NO_OF_TIMERS; j++)
    {
      //so for current minute does any timer set ?
      if (timerArray[tempTime][j] == 1)
      {
        digitalWrite(timerData.GPIOPin[j], timerData.activeHigh[j] ? HIGH : LOW); // Turn the relay on
        Serial.print(" >> RELAY: ");
        Serial.print(timerData.GPIOPin[j]);
        Serial.print(" ON. ");
        lastRelayStatus = true;
      }
      else
      {
        bool skipThisPin = false;
        for (int k = 0; k < NO_OF_TIMERS; k++)
        {
          if (timerArray[tempTime][k] == 1 && (timerData.GPIOPin[k] == timerData.GPIOPin[j])){
            skipThisPin = true;
            break;
          }
        }

        if(!skipThisPin){
          digitalWrite(timerData.GPIOPin[j], timerData.activeHigh[j] ? LOW : HIGH); // Turn the relay off
          Serial.print(" >> RELAY: ");
          Serial.print(timerData.GPIOPin[j]);
          Serial.print(" OFF. ");
        }
        else{
          Serial.print(" >> RELAY: ");
          Serial.print(timerData.GPIOPin[j]);
          Serial.print(" Skipped. ");
        }
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
