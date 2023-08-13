#ifndef RESTORE_FACTORY_SETTINGS_H
#define RESTORE_FACTORY_SETTINGS_H

#include <EEPROM.h>
#include "EEPROMAnything.h"

void restore_factory_settings()
{

  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, NULL);
  }
  EEPROM.commit(); // saves changes to flash

  struct TimerDataStruct TimerSet;

  for (int i = 0; i < 6; i++)
  {
    TimerSet.TimerOn[i] = true;
    TimerSet.HourOn[i] = (i * 4);
    TimerSet.MinuteOn[i] = 0;
    TimerSet.HourOff[i] = (i * 4) + 3;
    TimerSet.MinuteOff[i] = 0;
  }

  // TimerSet.HourOn[5] = '\0';
  // TimerSet.MinuteOn[5] = '\0';
  // TimerSet.TimerOn[5] = false;

  EEPROM_writeAnything(100, TimerSet);

  EEPROM_writeAnything(150, "YES");

  for (int i = 0; i < 512; i++)
  {
    char temp = EEPROM.read(i);
    Serial.print(temp);
  }

  Serial.println("");
  Serial.println("Factory Settings Restored");
}

#endif /* RESTORE_FACTORY_SETTINGS_H */
