#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <ESP8266WebServer.h>
#include "EEPROMAnything.h"
#include "Structs.h"
#include "common.h"
#include <ArduinoJson.h>
#include "restore_factory_settings.h"
#include <LittleFS.h>

void start_server();

void handleRoot();

void handle_client(struct CurrentTime, bool RelayStatus);

void handleSave();

void handleDetails();

void handleReset();

void handleAnyRequest();

void handleUpload();

#endif /* WEB_PORTAL_H */
