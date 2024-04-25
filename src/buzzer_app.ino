#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

/* Put your SSID & Password */
const char* ssid = "Wokwi-GUEST";  // Enter SSID here
const char* password = "";  //Enter Password here
const int WIFI_CHANNEL = 6; // Speeds up the connection in Wokwi

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");


//3 LEDS
int gameLEDS[] = { 14,27,26 };
//3 BUTTONS
int gameBTNS[] = { 15,5,4 };
//PIEZO BUZZER 
int buzzer = 13;
//Schalter für Gamemode
int schalter = 23;
//ALL DELAYS
int buzzerDelay = 100;
int blinkDelay = 500;
int resetDelay = 2000;
int anz = 3;
int kp = 0;

enum Gamemode {
  NORMAL,
  START_STOP,
  QUIZ_MODE
};

enum Gamemode gamemode;
bool resetTriggered;
bool clientConnected;
bool running;
bool resettet;
float time_stopped;
unsigned long timeStart;


// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  Serial.print("Connecting to WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, WIFI_CHANNEL);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.println(" Connected!");
}


void setup() {
  Serial.begin(115200);
  for (int i; i < anz; i++) {
    pinMode(gameLEDS[i], OUTPUT);
  }
  //This loops and sets up the Button pins to be INPUT_PULLUP in an array. Ground is HIGH so signal goes LOW on depress.
  for (int i; i < anz; i++) {
    pinMode(gameBTNS[i], INPUT_PULLUP);
  }
  pinMode(schalter, INPUT_PULLUP);
  
  initWiFi();
  initSPIFFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", createHTML());
    Serial.println("Client connected!");
    clientConnected = true;
  });

  server.on("/change_gamemode", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("gamemode", true)) {
      int gamemode_received = request->getParam("gamemode", true)->value().toInt();
      Serial.print("Gamemode-Change received: ");
      Serial.println(gamemode_received);
      switch (gamemode_received)
      {
      case NORMAL:
        gamemode = NORMAL;
        break;

      case START_STOP:
        gamemode = START_STOP;
        break;

      case QUIZ_MODE:
        gamemode = QUIZ_MODE;
        break;

      default:
        gamemode = NORMAL;
      }
    }
    else {
      Serial.println("ERROR: No gamemode parameter provided in request");
    }
    request->send(200, "text/html", createHTML());
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    resetTriggered = true;
    request->send(200, "text/html", createHTML());
  });

  //TODO: comment this in if not in emulator mode
  //server.serveStatic("/", SPIFFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(time_stopped));
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  server.addHandler(&events);

  // Start server
  server.begin();
  delay(100);
  Serial.println("Server jestachtet");

  gamemode = NORMAL;
  resetTriggered = false;
  clientConnected = false;
  running = false;
  resettet = true;
  time_stopped = 0.0;
  timeStart = 0;

  
}

void all_blink_and_buzz() {
  for (int i = 0; i < anz; i++) {
    digitalWrite(gameLEDS[i], HIGH);
  }
  tone(buzzer, 1300, 50);
  delay(blinkDelay);
  for (int i = 0; i < anz; i++) {
    digitalWrite(gameLEDS[i], LOW);
  }
}

void reset() {
  for (int i = 0; i < anz; i++) {
    digitalWrite(gameLEDS[i], LOW);
  }
  resettet = true;
}

void normal_game() {
  for (int i = 0; i < anz; i++) {
    //Press the button to intiate which button chimes in
    if (digitalRead(gameBTNS[i]) == LOW && resettet) {

      resettet = false;

      // This is the indicator light and buzzer sound confirmation
      digitalWrite(gameLEDS[i], HIGH);

      //Buzzer and light flicker
      // tone(buzzer, 1300, 50);
      // delay(buzzerDelay);
      // digitalWrite(gameLEDS[i], LOW);
      // tone(buzzer, 1300, 50);
      // delay(buzzerDelay);
      // digitalWrite(gameLEDS[i], HIGH);
      // tone(buzzer, 1300, 50);
      // delay(buzzerDelay);
      // digitalWrite(gameLEDS[i], LOW);
      // tone(buzzer, 1300, 50);
      // delay(buzzerDelay);
      // digitalWrite(gameLEDS[i], HIGH);
      // tone(buzzer, 1300, 50);

      Serial.print("gebuzzert: LEDpin: ");
      Serial.print(gameLEDS[i]);
      Serial.print(" Buzzerpin: ");
      Serial.println(gameBTNS[i]);

      if (!clientConnected) {
        delay(resetDelay);   
        reset();
        Serial.println("Zeit-Reset!");
        delay(500);
      }
      else {
        events.send("", "reset_enabled", millis());
      }
    }
  }
  if (digitalRead(schalter) == LOW) {
    gamemode = START_STOP;
    Serial.println("gewechselt-timer");
    delay(500);
  }
}

void start_stop_game() {
  for (int i = 0; i < anz; i++) {
   
    if (digitalRead(gameBTNS[i]) == LOW) {
      if (!running) {
        timeStart = millis();
        running = true;
        Serial.println("--Stoppuhr gestartet--");
        all_blink_and_buzz();
      }
      else {
        unsigned long elapsedTime = millis() - timeStart;
        running = false;
        all_blink_and_buzz();
        Serial.println("--Stoppuhr angehalten--");
        float elapsedInSec = elapsedTime/1000.0;
        Serial.print(elapsedInSec);
        Serial.print("s");
        Serial.println();
        time_stopped = elapsedInSec;
        events.send(String(time_stopped).c_str(), "timer_stopped", millis());
      }     
    }
  }
  if (digitalRead(schalter) == LOW) {
    gamemode = NORMAL;
    Serial.println("gewechselt-normal");
    delay(500);
  }
}

void loop() {
  if (gamemode == NORMAL) {
    normal_game();
  }
  else if (gamemode == START_STOP) {
    resettet = true;
    start_stop_game();
  }

  if (resetTriggered) {
    if (!resettet) {
      reset();
      Serial.println("manueller Reset!");
    }
    else {
      Serial.println("Kein Reset möglich");
    }
    resetTriggered = false;
  }
}

//This part is needed since Wokwi doesn't support SPIFFS
String createHTML() {
  String html = R"(
    <!DOCTYPE html>
    <html>

    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
        <title>Buzzerjeraet Jakob und Till</title>
        <style>
          html {
              font-family: Helvetica;
              display: inline-block;
              margin: 0px auto;
              text-align: center;
          }

          body {
              margin-top: 50px;
          }

          h1 {
              color: #444444;
              margin: 50px auto 30px;
          }

          h3 {
              color: #444444;
              margin-bottom: 50px;
          }

          .button {
              display: block;
              width: 80px;
              background-color: #3498db;
              border: none;
              color: white;
              padding: 13px 30px;
              text-decoration: none;
              font-size: 25px;
              margin: 0px auto 35px;
              cursor: pointer;
              border-radius: 4px;
          }

          .button-on {
              background-color: #3498db;
          }

          .button-on:active {
              background-color: #2980b9;
          }

          .button-off {
              background-color: #34495e;
          }

          .button-off:active {
              background-color: #2c3e50;
          }

          p {
              font-size: 14px;
              color: #888;
              margin-bottom: 10px;
          }
        </style>
    </head>

    <body>
        <div>
            <h1>Buzzer pour la Gang</h1>
            <h3>by Bongobrain und Till dem Boss</h3>
            <h4>Vergangene Zeit:</h4>
            <p id="elapsed-time">1s</p>
            <p>Gamemode</p>
            <select name="gamemode" id="gamemode">
              <option value="0" NORMAL_ACTIVE>Normal</option>
              <option value="1" START_STOP_ACTIVE>Start/Stop</option>
              <option value="2" QUIZ_MODE_ACTIVE>Quiz-Mode</option>
            </select>
            <button id="gamemode-button" type="button" onclick="change_gamemode() ">Bestätigen</button>
            <button id="reset-button" type="button" onclick="reset() " RESET_DISABLED> Reset</button>
        </div>
        <script>
          window.addEventListener('load', getTimer);

          function getTimer() {
              var xhr = new XMLHttpRequest();
              xhr.open("GET", "/readings", true);
              xhr.send();
          }

          function reset() {
              document.getElementById('reset-button').disabled = true;
              var xhr = new XMLHttpRequest();
              xhr.open("GET", "/reset", true);
              xhr.send();
          }

          function change_gamemode() {
            var params = "gamemode=" + document.getElementById("gamemode").value;
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/change_gamemode", true);
            xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');

            xhr.send(params);
          }

          if (!!window.EventSource) {
              var source = new EventSource('/events');

              source.addEventListener('open', function (e) {
                  console.log("Events Connected");
              }, false);

              source.addEventListener('error', function (e) {
                  if (e.target.readyState != EventSource.OPEN) {
                      console.log("Events Disconnected");
                  }
              }, false);

              source.addEventListener('message', function (e) {
                  console.log("message", e.data);
              }, false);

              source.addEventListener('reset_enabled', function (e) {
                  console.log("reset_enabled");
                  document.getElementById('reset-button').disabled = false;
              }, false); 

              source.addEventListener('timer_stopped', function (e) {
                  console.log("timer_stopped", e.data);
                  var time = e.data;
                  document.getElementById('elapsed-time').innerHTML = time + "s";
              }, false);
          }
        </script>
    </body>
    </html>
  )";
  switch (gamemode)
  {
    case START_STOP:
      html.replace("START_STOP_ACTIVE", "selected");
      html.replace("NORMAL_ACTIVE", "");
      html.replace("QUIZ_MODE_ACTIVE", "");
      break;
    case NORMAL:
      html.replace("NORMAL_ACTIVE", "selected");
      html.replace("START_STOP_ACTIVE", "");
      html.replace("QUIZ_MODE_ACTIVE", "");
      break;
    case QUIZ_MODE:
      html.replace("QUIZ_MODE_ACTIVE", "selected");
      html.replace("START_STOP_ACTIVE", "");
      html.replace("NORMAL_ACTIVE", "");
      break;
  }
  if (!resettet && !resetTriggered) {
    html.replace("RESET_DISABLED", "");
  }
  else {
    html.replace("RESET_DISABLED", "disabled");
  }
  return html;
}