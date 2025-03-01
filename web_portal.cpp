#include "web_portal.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

ESP8266WebServer server(80);

struct CurrentTime Present;
bool outputStatus;
bool sendingContent = false;
const short JSON_PAYLOAD_SIZE = 2048;
bool isSleeping = false;

void start_server()
{
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/settings", handleDetails);
  server.on("/reset", handleReset);
  server.on("/upload", handleUpload);
  server.onNotFound(handleAnyRequest);
  server.begin();
}

void handleUpload() {
  if (server.hasArg("html")) {
    String content = server.arg("html");
    File file = LittleFS.open("/index.html", "w");
    if (file) {
      file.print(content);
      file.close();
      server.send(200, "text/plain", "HTML content saved to LittleFS");
    } else {
      server.send(500, "text/plain", "Failed to save HTML content");
    }
  } else {
    server.send(400, "text/plain", "Missing or invalid parameter");
  }
}

void handleAnyRequest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200);
}

void handle_client(struct CurrentTime temptime, bool relayStatus) {
  Present = temptime;
  outputStatus = relayStatus;
  server.handleClient();

  if (sendingContent) {
    delay(10);
  }
}

void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// void goToSleep() {
//     Serial.println("😴 Entering Modem Sleep...");
//     isSleeping = true;
//     wifi_set_sleep_type(MODEM_SLEEP_T);
// }

// bool getIsSleeping() {
//   return isSleeping;
// }

void handleRoot() {

  // Serial.println("⏰ Wake-up: Received HTTP request");
  // isSleeping = false; // Mark that the ESP is awake

  sendingContent = true;
  addCORSHeaders();

  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  server.streamFile(file, "text/html");
  file.close();

  sendingContent = false;
}

void handleSave() {
  Serial.print("1 Free Memory: ");
  Serial.println(ESP.getFreeHeap());
  struct TimerDataStruct timerData;

  // Use a smaller JSON document size
  StaticJsonDocument<JSON_PAYLOAD_SIZE> jsonDocument; // Adjust size based on your payload

  // Parse JSON directly from the request body
  DeserializationError error = deserializeJson(jsonDocument, server.arg("plain"));
  Serial.println("Trying to Save:");
  Serial.println(server.arg("plain"));

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    server.send(400, "text/plain", "Bad Request: Invalid JSON data");
    return;
  }

  Serial.print("2 Free Memory: ");
  Serial.println(ESP.getFreeHeap());

  Serial.println("=========== Setting New values ===========");

  // Extract data from JSON
  for (int i = 0; i < NO_OF_TIMERS; i++) {
    timerData.hourOn[i] = jsonDocument["hourOn"][i];
    timerData.minuteOn[i] = jsonDocument["minuteOn"][i];
    timerData.secondOn[i] = jsonDocument["secondOn"][i];
    timerData.hourOff[i] = jsonDocument["hourOff"][i];
    timerData.minuteOff[i] = jsonDocument["minuteOff"][i];
    timerData.secondOff[i] = jsonDocument["secondOff"][i];
    timerData.timerOn[i] = jsonDocument["timerOn"][i];
    timerData.gpioPin[i] = jsonDocument["gpioPin"][i];
    timerData.activeHigh[i] = jsonDocument["activeHigh"][i];
  }

  timerData.allOff = jsonDocument["allOff"];
  timerData.timezone = jsonDocument["timezone"];
  strncpy(timerData.hostname, jsonDocument["hostname"], sizeof(timerData.hostname) - 1);
  timerData.hostname[sizeof(timerData.hostname) - 1] = '\0'; // Ensure null termination

  Serial.println("=========== Done Setting New values ===========");

  sendingContent = true;
  server.sendHeader("Content-Type", "text/html");
  addCORSHeaders();
  server.send(200, "text/html", "<html><head><meta http-equiv=\"refresh\" content=\"5\"></head><body>Saved! Page will refresh in 5 seconds.</body></html>");
  sendingContent = false;

  Serial.print("3 Free Memory: ");
  Serial.println(ESP.getFreeHeap());

  EEPROM_writeAnything(SETTINGS_LOCATION, timerData);
  Serial.print("Saved new settings");
  ESP.restart();
}

void handleReset() {
  restore_factory_settings();
  Serial.println("Something is corrupt so restoring factory settings");

  sendingContent = true;
  // Redirect to root ("/") after 5 seconds
  String html = "<html><head><meta http-equiv=\"refresh\" content=\"5;url=/\"></head><body>Success! Page will redirect to home in 5 seconds.</body></html>";
  addCORSHeaders();
  server.send(200, "text/html", html);
  sendingContent = false;

  ESP.restart();
}

void handleDetails() {
  struct TimerDataStruct timerSettings;
  EEPROM_readAnything(SETTINGS_LOCATION, timerSettings);

  // Use a smaller JSON document size
  StaticJsonDocument<JSON_PAYLOAD_SIZE> doc; // Adjust size based on your payload

  // Populate JSON document
  JsonArray hourOn = doc.createNestedArray("hourOn");
  JsonArray hourOff = doc.createNestedArray("hourOff");
  JsonArray minuteOn = doc.createNestedArray("minuteOn");
  JsonArray minuteOff = doc.createNestedArray("minuteOff");
  JsonArray secondOn = doc.createNestedArray("secondOn");
  JsonArray secondOff = doc.createNestedArray("secondOff");
  JsonArray timerOn = doc.createNestedArray("timerOn");
  JsonArray gpioPin = doc.createNestedArray("gpioPin");
  JsonArray activeHigh = doc.createNestedArray("activeHigh");

  for (int i = 0; i < NO_OF_TIMERS; i++) {
    hourOn.add(timerSettings.hourOn[i]);
    hourOff.add(timerSettings.hourOff[i]);
    minuteOn.add(timerSettings.minuteOn[i]);
    minuteOff.add(timerSettings.minuteOff[i]);
    secondOn.add(timerSettings.secondOn[i]);
    secondOff.add(timerSettings.secondOff[i]);
    timerOn.add(timerSettings.timerOn[i]);
    gpioPin.add(timerSettings.gpioPin[i]);
    activeHigh.add(timerSettings.activeHigh[i]);
  }

  doc["allOff"] = timerSettings.allOff;
  doc["timezone"] = timerSettings.timezone;
  doc["hostname"] = timerSettings.hostname;

  // Serialize JSON to a string
  String jsonData;
  serializeJson(doc, jsonData);
  Serial.println("Settings Read: ");
  Serial.println(jsonData);

  sendingContent = true;
  addCORSHeaders();
  server.send(200, "application/json", jsonData);
  sendingContent = false;
}