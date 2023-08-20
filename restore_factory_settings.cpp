#include "restore_factory_settings.h"

void restore_factory_settings()
{

  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, NULL);
  }
  EEPROM.commit(); // saves changes to flash

  struct TimerDataStruct timerSettings;

  for (int i = 0; i < NO_OF_TIMERS; i++)
  {
    timerSettings.timerOn[i] = true;
    timerSettings.hourOn[i] = (i * 4);
    timerSettings.minuteOn[i] = 0;
    timerSettings.hourOff[i] = (i * 4) + 3;
    timerSettings.minuteOff[i] = 0;
    timerSettings.activeHigh[i] = false;
    timerSettings.gpioPin[i] = 0;
  }

  timerSettings.allOff = false;
  timerSettings.timezone = 5.5;

  EEPROM_writeAnything(100, timerSettings);

  EEPROM_writeAnything(500, "YES");

  for (int i = 0; i < 512; i++)
  {
    char temp = EEPROM.read(i);
    Serial.print(temp);
  }

  Serial.println("");
  Serial.println("Factory Settings Restored");
}