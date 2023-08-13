#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <ESP8266WebServer.h>
#include "EEPROMAnything.h"
#include "Structs.h"

struct TimerDataStruct
{

    uint8_t HourOn[10];
    uint8_t MinuteOn[10];
    uint8_t HourOff[10];
    uint8_t MinuteOff[10];
    bool TimerOn[10];
    uint8_t GPIOPin[10];
    bool activeHigh[10];
};

void start_server();

void handleRoot();

void handle_client(struct CurrentTime, bool RelayStatus);

void handleSet();

#endif /* WEB_PORTAL_H */
