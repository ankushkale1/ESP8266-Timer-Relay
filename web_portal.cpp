#include "web_portal.h"
#include "common.h"

ESP8266WebServer server(80);

struct CurrentTime Present;
bool outputStatus;

const char TIMERS[] PROGMEM = R"=====(
                                          <div class="border p-3 mt-3">
                                            <div class="mb-3">
                                                <label class="form-label">Timer {index}</label>
                                            </div>
                                            <div class="mb-3">
                                              <label for="{Tmr}{index}GPIO" class="form-label">GPIO Pin</label>
                                              <input type="text" class="form-control" name="{Tmr}{index}GPIO" value='{TGPIO}' required>
                                            </div>
                                            <div class="mb-3">
                                              <label for="{Tmr}{index}pinstate" class="form-label">Relay Type</label>
                                              <input class="form-control" type="radio" name="{Tmr}{index}pinstate" value="on" {TPINSTATEON}> Active High
                                              <input class="form-control" type="radio" name="{Tmr}{index}pinstate" value="off" {TPINSTATEOFF}> Active Low
                                            </div>
                                            <div class="mb-3">
                                              <label for="{Tmr}{index}State" class="form-label">State</label>
                                              <input class="form-control" type="radio" name="{Tmr}{index}State" value="D" {CON}> ON
                                              <input class="form-control" type="radio" name="{Tmr}{index}State" value="O" {COF}> OFF
                                            </div>
                                            <div class="mb-3">
                                              <label for="{Tmr}{index}OnValue" class="form-label">Timer</label>
                                              <input type="time" name="{Tmr}{index}OnValue" id="{Tmr}{index}OnValue" value='{TON}' size=2 autofocus> 
                                              <label for="{Tmr}{index}OffValue" class="form-label">Timer</label>
                                              <input type="time" name="{Tmr}{index}OffValue" id="{Tmr}{index}OffValue" value='{TOF}' size=2 autofocus>
                                            </div>
                                          </div>
                              )=====";

void start_server()
{
  server.on("/", handleRoot);
  server.on("/set", handleSet);

  server.begin();
}

void handle_client(struct CurrentTime temptime, bool relayStatus)
{
  Present = temptime;
  outputStatus = relayStatus;
  server.handleClient();
}

void handleRoot()
{
  String timersHTML[NO_OF_TIMERS]; //actual html code for each timer
  String Page;
  char tempOnTime[NO_OF_TIMERS][6];
  char tempOffTime[NO_OF_TIMERS][6];
  struct TimerDataStruct timerSettings;

  EEPROM_readAnything(100, timerSettings);

//code to convert hour and minuts to html friendly string
  for (int i = 0; i < NO_OF_TIMERS; i++)
  {
    if (timerSettings.HourOn[i] < 10 && timerSettings.MinuteOn[i] < 10)
    {
      sprintf(tempOnTime[i], "0%d:0%d", timerSettings.HourOn[i], timerSettings.MinuteOn[i]);
    }
    else if (timerSettings.HourOn[i] < 10)
    {
      sprintf(tempOnTime[i], "0%d:%d", timerSettings.HourOn[i], timerSettings.MinuteOn[i]);
    }
    else if (timerSettings.MinuteOn[i] < 10)
    {
      sprintf(tempOnTime[i], "%d:0%d", timerSettings.HourOn[i], timerSettings.MinuteOn[i]);
    }
    else
    {
      sprintf(tempOnTime[i], "%d:%d", timerSettings.HourOn[i], timerSettings.MinuteOn[i]);
    }

    Serial.print(">> Formattted ON Time: ");
    Serial.println(tempOnTime[i]);
  }
  //tempOnTime[10][0] = '\0';

  for (int i = 0; i < NO_OF_TIMERS; i++)
  {
    if (timerSettings.HourOff[i] < 10 && timerSettings.MinuteOff[i] < 10)
    {
      sprintf(tempOffTime[i], "0%d:0%d", timerSettings.HourOff[i], timerSettings.MinuteOff[i]);
    }
    else if (timerSettings.HourOff[i] < 10)
    {
      sprintf(tempOffTime[i], "0%d:%d", timerSettings.HourOff[i], timerSettings.MinuteOff[i]);
    }
    else if (timerSettings.MinuteOff[i] < 10)
    {
      sprintf(tempOffTime[i], "%d:0%d", timerSettings.HourOff[i], timerSettings.MinuteOff[i]);
    }
    else
    {
      sprintf(tempOffTime[i], "%d:%d", timerSettings.HourOff[i], timerSettings.MinuteOff[i]);
    }

    Serial.print(">> Formattted OFF Time: ");
    Serial.println(tempOnTime[i]);
  }
  //tempOffTime[10][0] = '\0';
//

  //prepare each timer html template with actual values
  for(int i=0;i<NO_OF_TIMERS;i++){
      if (timerSettings.TimerOn[i])
      {
        timersHTML[i] = FPSTR(TIMERS);
        timersHTML[i].replace("{Tmr}", F("Timer"));
        timersHTML[i].replace("{index}", String(i).c_str());
        timersHTML[i].replace("{CON}", F("checked"));
        timersHTML[i].replace("{COF}", F(""));
        timersHTML[i].replace("{TON}", tempOnTime[i]);
        timersHTML[i].replace("{TOF}", tempOffTime[i]);
        timersHTML[i].replace("{TGPIO}", String(timerSettings.GPIOPin[i]).c_str());
        timersHTML[i].replace("{TPINSTATEON}", timerSettings.activeHigh[i] ? "checked" : "");
        timersHTML[i].replace("{TPINSTATEOFF}", !timerSettings.activeHigh[i] ? "checked" : "");
      }
      else
      {
        timersHTML[i] = FPSTR(TIMERS);
        timersHTML[i].replace("{Tmr}", F("Timer"));
        timersHTML[i].replace("{index}", String(i).c_str());
        timersHTML[i].replace("{CON}", F(""));
        timersHTML[i].replace("{COF}", F("checked"));
        timersHTML[i].replace("{TON}", tempOnTime[i]);
        timersHTML[i].replace("{TOF}", tempOffTime[i]);
        timersHTML[i].replace("{TGPIO}", String(timerSettings.GPIOPin[i]).c_str());
        timersHTML[i].replace("{TPINSTATEON}", timerSettings.activeHigh[i] ? "checked" : "");
        timersHTML[i].replace("{TPINSTATEOFF}", !timerSettings.activeHigh[i] ? "checked" : "");
      }
  }
  //

  //current time in char so that we can display it on page
  char tempTime[6];
  if (Present.Minute < 10 && Present.Second < 10)
  {
    sprintf(tempTime, "0%d:0%d", Present.Minute, Present.Second);
  }
  else if (Present.Minute < 10)
  {
    sprintf(tempTime, "0%d:%d", Present.Minute, Present.Second);
  }
  else if (Present.Second < 10)
  {
    sprintf(tempTime, "%d:0%d", Present.Minute, Present.Second);
  }
  else
  {
    sprintf(tempTime, "%d:%d", Present.Minute, Present.Second);
  }
  
  //

  Page = F("<!DOCTYPE html>"
           "<html lang=\"en\">"
           "<head>"
           "<title>Relay Timer Server</title>"
           "<meta charset=\"utf-8\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
           "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\">"
           "<style>"
           ".button{background-color:#DD4814;border:none;color:white;padding:10px 20px;text-align:center;text-decoration:none;display:inline-block;font-size:16px;border-radius:5px;cursor:pointer;width:100%;}.button:hover{background-color:#FF6F18;}"
           "</style>"
           "</head>"
           "<body>"
           "<div class=\"container mt-5\">"
           "<h1>Daily Repeat Timer Relay</h1>"
           "<h2>The time is ");
  
  Page += Present.Hour;
  Page += ":";
  Page += tempTime;
  Page += F(" </h2>");
  Page += F("<h3>Enter your timer on/off values in format (05:45)</h3>"
            "<h4>Do not create timers that pan over 2 days, for example [22:00]until[02:00]</h4>"
            "<h4>Create 2 different timers, [22:00]until[24:00] and [00:00]until[02:00]</h4>"
            "<form id=\"Timerform\">"
            "<div class=\"border p-3 mt-3\">");

  for(int i=0; i < NO_OF_TIMERS; i++){
    Page += timersHTML[i];
    //Page.replace("{index}", String(i).c_str());
  }

  Page += F("</div>"
            "</form>"
            "<div>"
            "<input type=\"button\" name='Submit' value='Submit' class='button' id='submitButton'/>"
            "</div>"
            "</div>"
            "<script src=\"https://code.jquery.com/jquery-3.5.1.slim.min.js\"></script>"
            "<script src=\"https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.1/dist/umd/popper.min.js\"></script>"
            "<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js\"></script>"
            "<script>$('#submitButton').click(function() {"
            "var formData = $('#Timerform').serialize();"
            "$.post('/set', formData, function(response) {"
             "   console.log(response);"
            "});"
            "});</script>"
            "</body>"
            "</html>");

  server.send(200, "text/html", Page);
}

void handleSet()
{
  struct TimerDataStruct WebPortalTimerSet;
  String tempTime;
  String tempHourString;
  String tempMinuteString;

  Serial.println("=========== Setting New values ===========");

  for(int i=0; i<NO_OF_TIMERS; i++){
    String stateParam = F("Timer{index}State");
    stateParam.replace("{index}", String(i).c_str());

    if (server.arg(stateParam) == "D")
    {
      WebPortalTimerSet.TimerOn[i] = true;
    }
    else if (server.arg(stateParam) == "O")
    {
      WebPortalTimerSet.TimerOn[i] = false;
    }

    String onValueParam = F("Timer{index}OnValue");
    onValueParam.replace("{index}", String(i).c_str());

    String offValueParam = F("Timer{index}OffValue");
    offValueParam.replace("{index}", String(i).c_str());

    //if (server.arg(onValueParam).toInt() && server.arg(offValueParam).toInt()){
    tempTime = server.arg(onValueParam);
    Serial.print("Timer ");
    Serial.print(i);
    Serial.print(" on value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOn and MinuteOn form
    WebPortalTimerSet.HourOn[i] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOn[i] = tempMinuteString.toInt();

    tempTime = server.arg(offValueParam);
    Serial.print("Timer ");
    Serial.print(i);
    Serial.print(" off value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOff and MinuteOff form
    WebPortalTimerSet.HourOff[i] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOff[i] = tempMinuteString.toInt();
    //}

    String gpioPin = F("Timer{index}GPIO");
    gpioPin.replace("{index}", String(i).c_str());
    WebPortalTimerSet.GPIOPin[i] = server.arg(gpioPin).toInt();
    Serial.print("Timer ");
    Serial.print(i);
    Serial.print(" GPIO value Set for ");
    Serial.println(WebPortalTimerSet.GPIOPin[i]);

    String pintype = F("Timer{index}pinstate");
    pintype.replace("{index}", String(i).c_str());
    if (server.arg(pintype) == "on")
    {
      WebPortalTimerSet.activeHigh[i] = true;
    }else{
      WebPortalTimerSet.activeHigh[i] = false;
    }
    Serial.print("Timer ");
    Serial.print(i);
    Serial.print(" PinState value Set for ");
    Serial.println(WebPortalTimerSet.activeHigh[i]);

    Serial.println("=========== Done: Setting New values ===========");
  }

  String html = "<html><head><meta http-equiv=\"refresh\" content=\"5\"></head><body>Success! Page will reload in 5 seconds.</body></html>";
  server.send(200, "text/html", html);

  EEPROM_writeAnything(100, WebPortalTimerSet);
  ESP.restart();
}
