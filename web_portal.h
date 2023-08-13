#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <ESP8266WebServer.h>
#include "EEPROMAnything.h"
#include "Structs.h"

struct TimerDataStruct
{

    uint8_t HourOn[5];
    uint8_t MinuteOn[5];
    uint8_t HourOff[5];
    uint8_t MinuteOff[5];
    bool TimerOn[7];
};

void start_server();

void handleRoot();

void handle_client(struct CurrentTime, bool RelayStatus);

void handleSet();

#endif /* WEB_PORTAL_H */
