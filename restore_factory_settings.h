#ifndef RESTORE_FACTORY_SETTINGS_H
#define RESTORE_FACTORY_SETTINGS_H

#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "common.h"

void restore_factory_settings();

#define OK_LOCATION 1010
#define SETTINGS_LOCATION 100
#define EEPROM_SIZE 1024

#endif /* RESTORE_FACTORY_SETTINGS_H */
