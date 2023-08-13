#include "web_portal.h"

ESP8266WebServer server(80);

struct CurrentTime Present;
bool outputStatus;

const char FIELDSET[] PROGMEM = "</fieldset><fieldset>";

const char TIMERS[] PROGMEM = R"=====(
                                          <input type="radio" name="{Tmr}State" value="D" {CON}> ON
                                          <input type="radio" name="{Tmr}State" value="O" {COF}> OFF &nbsp &nbsp
                                          <input type="text" name="{Tmr}OnValue" id="{Tmr}OnValue" value='{TON}' size=2 autofocus> until <input type="text" name="{Tmr}OffValue" id="{Tmr}OffValue" value='{TOF}' size=2 autofocus>
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

  String Timer1SetHTML;
  String Timer2SetHTML;
  String Timer3SetHTML;
  String Timer4SetHTML;
  String Timer5SetHTML;
  String DayLtSaveSetHTML;
  String Page;
  char tempOnTime[6][6];
  char tempOffTime[6][6];
  struct TimerDataStruct TimerSettings;

  EEPROM_readAnything(100, TimerSettings);

  for (int i = 0; i < 6; i++)
  {
    if (TimerSettings.HourOn[i] < 10 && TimerSettings.MinuteOn[i] < 10)
    {
      sprintf(tempOnTime[i], "0%d:0%d", TimerSettings.HourOn[i], TimerSettings.MinuteOn[i]);
    }
    else if (TimerSettings.HourOn[i] < 10)
    {
      sprintf(tempOnTime[i], "0%d:%d", TimerSettings.HourOn[i], TimerSettings.MinuteOn[i]);
    }
    else if (TimerSettings.MinuteOn[i] < 10)
    {
      sprintf(tempOnTime[i], "%d:0%d", TimerSettings.HourOn[i], TimerSettings.MinuteOn[i]);
    }
    else
    {
      sprintf(tempOnTime[i], "%d:%d", TimerSettings.HourOn[i], TimerSettings.MinuteOn[i]);
    }
  }
  tempOnTime[6][0] = '\0';

  for (int i = 0; i < 6; i++)
  {
    if (TimerSettings.HourOff[i] < 10 && TimerSettings.MinuteOff[i] < 10)
    {
      sprintf(tempOffTime[i], "0%d:0%d", TimerSettings.HourOff[i], TimerSettings.MinuteOff[i]);
    }
    else if (TimerSettings.HourOff[i] < 10)
    {
      sprintf(tempOffTime[i], "0%d:%d", TimerSettings.HourOff[i], TimerSettings.MinuteOff[i]);
    }
    else if (TimerSettings.MinuteOff[i] < 10)
    {
      sprintf(tempOffTime[i], "%d:0%d", TimerSettings.HourOff[i], TimerSettings.MinuteOff[i]);
    }
    else
    {
      sprintf(tempOffTime[i], "%d:%d", TimerSettings.HourOff[i], TimerSettings.MinuteOff[i]);
    }
  }
  tempOffTime[6][0] = '\0';

  if (TimerSettings.TimerOn[0])
  {
    Timer1SetHTML = FPSTR(TIMERS);
    Timer1SetHTML.replace("{Tmr}", F("Timer1"));
    Timer1SetHTML.replace("{CON}", F("checked"));
    Timer1SetHTML.replace("{COF}", F(""));
    Timer1SetHTML.replace("{TON}", tempOnTime[0]);
    Timer1SetHTML.replace("{TOF}", tempOffTime[0]);
  }
  else
  {
    Timer1SetHTML = FPSTR(TIMERS);
    Timer1SetHTML.replace("{Tmr}", F("Timer1"));
    Timer1SetHTML.replace("{CON}", F(""));
    Timer1SetHTML.replace("{COF}", F("checked"));
    Timer1SetHTML.replace("{TON}", tempOnTime[0]);
    Timer1SetHTML.replace("{TOF}", tempOffTime[0]);
  }

  if (TimerSettings.TimerOn[1])
  {
    Timer2SetHTML = FPSTR(TIMERS);
    Timer2SetHTML.replace("{Tmr}", F("Timer2"));
    Timer2SetHTML.replace("{CON}", F("checked"));
    Timer2SetHTML.replace("{COF}", F(""));
    Timer2SetHTML.replace("{TON}", tempOnTime[1]);
    Timer2SetHTML.replace("{TOF}", tempOffTime[1]);
  }
  else
  {
    Timer2SetHTML = FPSTR(TIMERS);
    Timer2SetHTML.replace("{Tmr}", F("Timer2"));
    Timer2SetHTML.replace("{CON}", F(""));
    Timer2SetHTML.replace("{COF}", F("checked"));
    Timer2SetHTML.replace("{TON}", tempOnTime[1]);
    Timer2SetHTML.replace("{TOF}", tempOffTime[1]);
  }

  if (TimerSettings.TimerOn[2])
  {
    Timer3SetHTML = FPSTR(TIMERS);
    Timer3SetHTML.replace("{Tmr}", F("Timer3"));
    Timer3SetHTML.replace("{CON}", F("checked"));
    Timer3SetHTML.replace("{COF}", F(""));
    Timer3SetHTML.replace("{TON}", tempOnTime[2]);
    Timer3SetHTML.replace("{TOF}", tempOffTime[2]);
  }
  else
  {
    Timer3SetHTML = FPSTR(TIMERS);
    Timer3SetHTML.replace("{Tmr}", F("Timer3"));
    Timer3SetHTML.replace("{CON}", F(""));
    Timer3SetHTML.replace("{COF}", F("checked"));
    Timer3SetHTML.replace("{TON}", tempOnTime[2]);
    Timer3SetHTML.replace("{TOF}", tempOffTime[2]);
  }

  if (TimerSettings.TimerOn[3])
  {
    Timer4SetHTML = FPSTR(TIMERS);
    Timer4SetHTML.replace("{Tmr}", F("Timer4"));
    Timer4SetHTML.replace("{CON}", F("checked"));
    Timer4SetHTML.replace("{COF}", F(""));
    Timer4SetHTML.replace("{TON}", tempOnTime[3]);
    Timer4SetHTML.replace("{TOF}", tempOffTime[3]);
  }
  else
  {
    Timer4SetHTML = FPSTR(TIMERS);
    Timer4SetHTML.replace("{Tmr}", F("Timer4"));
    Timer4SetHTML.replace("{CON}", F(""));
    Timer4SetHTML.replace("{COF}", F("checked"));
    Timer4SetHTML.replace("{TON}", tempOnTime[3]);
    Timer4SetHTML.replace("{TOF}", tempOffTime[3]);
  }

  if (TimerSettings.TimerOn[4])
  {
    Timer5SetHTML = FPSTR(TIMERS);
    Timer5SetHTML.replace("{Tmr}", F("Timer5"));
    Timer5SetHTML.replace("{CON}", F("checked"));
    Timer5SetHTML.replace("{COF}", F(""));
    Timer5SetHTML.replace("{TON}", tempOnTime[4]);
    Timer5SetHTML.replace("{TOF}", tempOffTime[4]);
  }
  else
  {
    Timer5SetHTML = FPSTR(TIMERS);
    Timer5SetHTML.replace("{Tmr}", F("Timer5"));
    Timer5SetHTML.replace("{CON}", F(""));
    Timer5SetHTML.replace("{COF}", F("checked"));
    Timer5SetHTML.replace("{TON}", tempOnTime[4]);
    Timer5SetHTML.replace("{TOF}", tempOffTime[4]);
  }

  if (TimerSettings.TimerOn[5])
  {
    DayLtSaveSetHTML = F("<input type=\"radio\" name=\"DayLtSaveState\" value=\"D\" checked> ON"
                         "<input type=\"radio\" name=\"DayLtSaveState\" value=\"O\"> OFF");
  }
  else
  {
    DayLtSaveSetHTML = F("<input type=\"radio\" name=\"DayLtSaveState\" value=\"D\"> ON"
                         "<input type=\"radio\" name=\"DayLtSaveState\" value=\"O\" checked> OFF");
  }

  Page = F("<!DOCTYPE html>"
           "<html lang=\"en\">"
           "<head>"
           "<TITLE>Relay Timer Server</TITLE>"
           "<meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
           "</head>"
           "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>"
           "<style> .button { background-color: #4CAF50; border: on; color: white; padding: 6px 25px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}</style>"
           "<body>"
           "<h1>Daily Repeat Timer Relay</h1>"
           "<h2>The time is ");
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
  Page += Present.Hour;
  Page += ":";
  Page += tempTime;
  Page += F(" and the relay output is ");
  if (outputStatus)
  {
    Page += F("on</h2>");
  }
  else
  {
    Page += F("off</h2>");
  }
  Page += F("<h3>Enter your timer on/off values in format (05:45)</h3>"
            "<h4>Do not create timers that pan over 2 days, for example [22:00]until[02:00]</h4>"
            "<h4>Create 2 different timers, [22:00]until[24:00] and [00:00]until[02:00]</h4>"
            "<form id=\"Timerform\">"
            "<fieldset>"
            "<legend>Timer 1</legend>");
  Page += Timer1SetHTML;
  Page += FPSTR(FIELDSET);
  Page += "<legend>Timer 2</legend>";
  Page += Timer2SetHTML;
  Page += FPSTR(FIELDSET);
  Page += "<legend>Timer 3</legend>";
  Page += Timer3SetHTML;
  Page += FPSTR(FIELDSET);
  Page += "<legend>Timer 4</legend>";
  Page += Timer4SetHTML;
  Page += FPSTR(FIELDSET);
  Page += "<legend>Timer 5</legend>";
  Page += Timer5SetHTML;
  Page += FPSTR(FIELDSET);
  Page += "<legend>Day Light Savings</legend>";
  Page += DayLtSaveSetHTML;
  Page += F("</fieldset>"
            "<form>"
            "<fieldset>"
            "<legend>Actions</legend>"
            "<div>"
            "<br>"
            "<button id=\"set_button\" class=\"button\">Set</button>"
            "</div>"
            "</fieldset>"
            "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script><script>"
            "var Timer1State; var Timer1OnValue; var Timer1OffValue; var Timer2State; var Timer2OnValue; var Timer2OffValue; var Timer3State; var Timer3OnValue; var Timer3OffValue; var Timer4State; var Timer4OnValue; var Timer4OffValue; var Timer5State; var Timer5OnValue; var Timer5OffValue; $('#set_button').click(function(OnEvent){ OnEvent.preventDefault();"
            "Timer1State = $('input[name=Timer1State]:checked', '#Timerform').val(); Timer1OnValue = $('#Timer1OnValue').val(); Timer1OffValue = $('#Timer1OffValue').val(); Timer2State = $('input[name=Timer2State]:checked', '#Timerform').val(); Timer2OnValue = $('#Timer2OnValue').val(); Timer2OffValue = $('#Timer2OffValue').val();  Timer3State = $('input[name=Timer3State]:checked', '#Timerform').val(); Timer3OnValue = $('#Timer3OnValue').val(); Timer3OffValue = $('#Timer3OffValue').val(); Timer4State = $('input[name=Timer4State]:checked', '#Timerform').val();  Timer4OnValue = $('#Timer4OnValue').val(); Timer4OffValue = $('#Timer4OffValue').val(); Timer5State = $('input[name=Timer5State]:checked', '#Timerform').val();  Timer5OnValue = $('#Timer5OnValue').val(); Timer5OffValue = $('#Timer5OffValue').val(); DayLtSaveState = $('input[name=DayLtSaveState]:checked', '#Timerform').val();"
            "$.get('/set?Timer1State=' + Timer1State + '&Timer1OnValue=' + Timer1OnValue + '&Timer1OffValue=' + Timer1OffValue + '&Timer2State=' + Timer2State + '&Timer2OnValue=' + Timer2OnValue + '&Timer2OffValue=' + Timer2OffValue + '&Timer3State=' + Timer3State + '&Timer3OnValue=' + Timer3OnValue + '&Timer3OffValue=' + Timer3OffValue + '&Timer4State=' + Timer4State + '&Timer4OnValue=' + Timer4OnValue + '&Timer4OffValue=' + Timer4OffValue + '&Timer5State=' + Timer5State + '&Timer5OnValue=' + Timer5OnValue + '&Timer5OffValue=' + Timer5OffValue + '&DayLtSaveState=' + DayLtSaveState, function(TimerState){ console.log(TimerState); }); });"
            "</script>"
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

  if (server.arg("Timer1State") == "D")
  {
    WebPortalTimerSet.TimerOn[0] = true;
  }
  else if (server.arg("Timer1State") == "O")
  {
    WebPortalTimerSet.TimerOn[0] = false;
  }

  if (server.arg("Timer2State") == "D")
  {
    WebPortalTimerSet.TimerOn[1] = true;
  }
  else if (server.arg("Timer2State") == "O")
  {
    WebPortalTimerSet.TimerOn[1] = false;
  }

  if (server.arg("Timer3State") == "D")
  {
    WebPortalTimerSet.TimerOn[2] = true;
  }
  else if (server.arg("Timer3State") == "O")
  {
    WebPortalTimerSet.TimerOn[2] = false;
  }

  if (server.arg("Timer4State") == "D")
  {
    WebPortalTimerSet.TimerOn[3] = true;
  }
  else if (server.arg("Timer4State") == "O")
  {
    WebPortalTimerSet.TimerOn[3] = false;
  }

  if (server.arg("Timer5State") == "D")
  {
    WebPortalTimerSet.TimerOn[4] = true;
  }
  else if (server.arg("Timer5State") == "O")
  {
    WebPortalTimerSet.TimerOn[4] = false;
  }

  if (server.arg("DayLtSaveState") == "D")
  {
    WebPortalTimerSet.TimerOn[5] = true;
  }
  else if (server.arg("DayLtSaveState") == "O")
  {
    WebPortalTimerSet.TimerOn[5] = false;
  }

  if (server.arg("Timer1OnValue").toInt() && server.arg("Timer1OffValue").toInt())
  {

    tempTime = server.arg("Timer1OnValue");
    Serial.print("Timer 1 on value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOn and MinuteOn form
    WebPortalTimerSet.HourOn[0] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOn[0] = tempMinuteString.toInt();

    tempTime = server.arg("Timer1OffValue");
    Serial.print("Timer 1 off value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOff and MinuteOff form
    WebPortalTimerSet.HourOff[0] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOff[0] = tempMinuteString.toInt();
  }

  if (server.arg("Timer2OnValue").toInt() && server.arg("Timer2OffValue").toInt())
  {
    tempTime = server.arg("Timer2OnValue");
    Serial.print("Timer 2 on value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOn and MinuteOn form
    WebPortalTimerSet.HourOn[1] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOn[1] = tempMinuteString.toInt();

    tempTime = server.arg("Timer2OffValue");
    Serial.print("Timer 2 off value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOff and MinuteOff form
    WebPortalTimerSet.HourOff[1] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOff[1] = tempMinuteString.toInt();
  }

  if (server.arg("Timer3OnValue").toInt() && server.arg("Timer3OffValue").toInt())
  {
    tempTime = server.arg("Timer3OnValue");
    Serial.print("Timer 3 on value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOn and MinuteOn form
    WebPortalTimerSet.HourOn[2] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOn[2] = tempMinuteString.toInt();

    tempTime = server.arg("Timer3OffValue");
    Serial.print("Timer 3 off value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOff and MinuteOff form
    WebPortalTimerSet.HourOff[2] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOff[2] = tempMinuteString.toInt();
  }

  if (server.arg("Timer4OnValue").toInt() && server.arg("Timer4OffValue").toInt())
  {
    tempTime = server.arg("Timer4OnValue");
    Serial.print("Timer 4 on value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOn and MinuteOn form
    WebPortalTimerSet.HourOn[3] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOn[3] = tempMinuteString.toInt();

    tempTime = server.arg("Timer4OffValue");
    Serial.print("Timer 4 off value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOff and MinuteOff form
    WebPortalTimerSet.HourOff[3] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOff[3] = tempMinuteString.toInt();
  }

  if (server.arg("Timer5OnValue").toInt() && server.arg("Timer5OffValue").toInt())
  {
    tempTime = server.arg("Timer5OnValue");
    Serial.print("Timer 5 on value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOn and MinuteOn form
    WebPortalTimerSet.HourOn[4] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOn[4] = tempMinuteString.toInt();

    tempTime = server.arg("Timer5OffValue");
    Serial.print("Timer 5 off value Set for ");
    Serial.println(tempTime);
    tempHourString = tempTime.substring(0, 2); // substring is used to split the data in HourOff and MinuteOff form
    WebPortalTimerSet.HourOff[4] = tempHourString.toInt();
    tempMinuteString = tempTime.substring(3, 5);
    WebPortalTimerSet.MinuteOff[4] = tempMinuteString.toInt();
  }

  EEPROM_writeAnything(100, WebPortalTimerSet);
  ESP.restart();
}
