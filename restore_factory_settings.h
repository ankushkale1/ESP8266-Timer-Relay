#ifndef RESTORE_FACTORY_SETTINGS_H
#define RESTORE_FACTORY_SETTINGS_H

#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "common.h"

void restore_factory_settings()
{

  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, NULL);
  }
  EEPROM.commit(); // saves changes to flash

  struct TimerDataStruct TimerSet;

  for (int i = 0; i < NO_OF_TIMERS; i++)
  {
    TimerSet.TimerOn[i] = true;
    TimerSet.HourOn[i] = (i * 4);
    TimerSet.MinuteOn[i] = 0;
    TimerSet.HourOff[i] = (i * 4) + 3;
    TimerSet.MinuteOff[i] = 0;
    TimerSet.activeHigh[i] = false;
    TimerSet.GPIOPin[i] = 0;
  }

  // TimerSet.HourOn[5] = '\0';
  // TimerSet.MinuteOn[5] = '\0';
  // TimerSet.TimerOn[5] = false;

  EEPROM_writeAnything(100, TimerSet);

  EEPROM_writeAnything(500, "YES");

  for (int i = 0; i < 512; i++)
  {
    char temp = EEPROM.read(i);
    Serial.print(temp);
  }

  Serial.println("");
  Serial.println("Factory Settings Restored");
}

#endif /* RESTORE_FACTORY_SETTINGS_H */
