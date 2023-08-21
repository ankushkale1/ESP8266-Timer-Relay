#include "web_portal.h"

ESP8266WebServer server(80);

struct CurrentTime Present;
bool outputStatus;
bool sendingContent = false;

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

void handleUpload(){
  if (server.hasArg("html")) {
      String content = server.arg("html");
      File file = LittleFS.open("/index.html", "w"); //SPIFFS.open("/index.html", "w");
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
    // Set CORS headers to allow requests from any origin
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200); // Respond with a status code
}

void handle_client(struct CurrentTime temptime, bool relayStatus)
{
  Present = temptime;
  outputStatus = relayStatus;
  server.handleClient();

  // If content is being sent, give the ESP8266 a little time to process it
  if (sendingContent) {
    delay(10);
  }
}

void addCORSHeaders(){
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleRoot()
{
  sendingContent = true;
  addCORSHeaders();
  // server.setContentLength(page_len);
  // server.send(200, "text/html", myPage);

  File file = LittleFS.open("/index.html", "r"); //SPIFFS.open("/index.html", "r");
  if (!file) {
      server.send(404, "text/plain", "File not found");
      return;
  }

  server.streamFile(file, "text/html");
  file.close();

  sendingContent = false;
}

void handleSave(){
  struct TimerDataStruct timerData;

  DynamicJsonDocument jsonDocument(1536);
  String jsonStr = server.arg("plain");

  Serial.print("Posted Json Data: ");
  Serial.println(jsonStr);

  DeserializationError error = deserializeJson(jsonDocument, jsonStr);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    server.send(400, "text/plain", "Bad Request: Invalid JSON data");
    return;
  }

  Serial.println("=========== Setting New values ===========");

  for (int i = 0; i < NO_OF_TIMERS; i++) {
    timerData.hourOn[i] = jsonDocument["hourOn"][i];
    timerData.minuteOn[i] = jsonDocument["minuteOn"][i];
    timerData.hourOff[i] = jsonDocument["hourOff"][i];
    timerData.minuteOff[i] = jsonDocument["minuteOff"][i];
    timerData.timerOn[i] = jsonDocument["timerOn"][i];
    timerData.gpioPin[i] = jsonDocument["gpioPin"][i];
    timerData.activeHigh[i] = jsonDocument["activeHigh"][i];
  }

  timerData.allOff = jsonDocument["allOff"];
  timerData.timezone = jsonDocument["timezone"];
  strcpy(timerData.hostname,jsonDocument["hostname"]);

  Serial.println("=========== Done Setting New values ===========");

  sendingContent = true;
  server.sendHeader("Content-Type", "text/html");
  addCORSHeaders();
  server.send(200, "text/plain", "Success");
  sendingContent = false;

  EEPROM_writeAnything(100, timerData);
  ESP.restart();
}

void handleReset(){
  restore_factory_settings();
  Serial.println("Something is currupt so restoring factory settings");

  sendingContent = true;
  String html = "<html><head><meta http-equiv=\"refresh\" content=\"5\"></head><body>Success! page will reload in 5 seconds.</body></html>";
  addCORSHeaders();
  server.send(200, "text/html", html);
  sendingContent = false;

  ESP.restart();
}

void handleDetails()
{
  struct TimerDataStruct timerSettings;
  EEPROM_readAnything(100, timerSettings);

  DynamicJsonDocument doc(1536);
  JsonArray hourOn = doc.createNestedArray("hourOn");
  JsonArray hourOff = doc.createNestedArray("hourOff");
  JsonArray minuteOn = doc.createNestedArray("minuteOn");
  JsonArray minuteOff = doc.createNestedArray("minuteOff");
  JsonArray timerOn = doc.createNestedArray("timerOn");
  JsonArray gpioPin = doc.createNestedArray("gpioPin");
  JsonArray activeHigh = doc.createNestedArray("activeHigh");

  for(int i=0;i<NO_OF_TIMERS;i++){
    hourOn.add(timerSettings.hourOn[i]);
    hourOff.add(timerSettings.hourOff[i]);
    minuteOn.add(timerSettings.minuteOn[i]);
    minuteOff.add(timerSettings.minuteOff[i]);
    timerOn.add(timerSettings.timerOn[i]);
    gpioPin.add(timerSettings.gpioPin[i]);
    activeHigh.add(timerSettings.activeHigh[i]);
  }

  doc["allOff"] = timerSettings.allOff;
  doc["timezone"] = timerSettings.timezone;  
  doc["hostname"] = timerSettings.hostname;

  String jsonData;
  serializeJson(doc, jsonData);

  sendingContent = true;
  addCORSHeaders();
  server.send(200, "application/json", jsonData);
  sendingContent = false;
}
