#ifndef GET_TIME_H
#define GET_TIME_H

#include "NTPClient.h"
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"
#include "Structs.h"

// NZ timezone is utc+12:00 and utc+13:00 during daylight savings
// daylight savings for 2021 in NZ is between 26 September & 4 April

String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup_time(float timeZone)
{

  long utcOffsetInSeconds = timeZone * 3600;
  timeClient.setTimeOffset(utcOffsetInSeconds);
  timeClient.begin();
}

struct CurrentTime Current_Time()
{

  struct CurrentTime Current;
  char tempTime[6];

  timeClient.update();
  Current.Hour = timeClient.getHours();
  Current.Minute = timeClient.getMinutes();
  Current.Second = timeClient.getSeconds();
  Current.Day = timeClient.getDay();

  if (Current.Hour < 10 && Current.Minute < 10)
  {
    sprintf(tempTime, "0%d:0%d", Current.Hour, Current.Minute);
  }
  else if (Current.Hour < 10)
  {
    sprintf(tempTime, "0%d:%d", Current.Hour, Current.Minute);
  }
  else if (Current.Minute < 10)
  {
    sprintf(tempTime, "%d:0%d", Current.Hour, Current.Minute);
  }
  else
  {
    sprintf(tempTime, "%d:%d", Current.Hour, Current.Minute);
  }
  Serial.print(weekDays[Current.Day]);
  Serial.print(" - ");
  Serial.println(tempTime);

  return Current;
}

#endif /* GET_TIME_H */
