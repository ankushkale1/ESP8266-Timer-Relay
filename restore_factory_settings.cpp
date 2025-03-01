#include "restore_factory_settings.h"

void restore_factory_settings()
{

  for (int i = 0; i < EEPROM_SIZE; i++)
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
    timerSettings.secondOn[i] = 0;
    timerSettings.hourOff[i] = (i * 4) + 3;
    timerSettings.minuteOff[i] = 0;
    timerSettings.secondOff[i] = 0;
    timerSettings.activeHigh[i] = false;
    timerSettings.gpioPin[i] = 0;
  }

  timerSettings.allOff = false;
  timerSettings.timezone = 5.5;
  strcpy(timerSettings.hostname, "relay");

  EEPROM_writeAnything(SETTINGS_LOCATION, timerSettings);

  EEPROM_writeAnything(OK_LOCATION, "YES");

  //Serial.print("Raw Data written in EEPROM: ");
  for (int i = 0; i < 1024; i++)
  {
    char temp = EEPROM.read(i);
    Serial.println(temp);
  }

  Serial.println("");
  Serial.println("Factory Settings Restored");
}