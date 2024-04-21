#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

/* Put your SSID & Password */
const char* ssid = "BuzzerBongoTill";  // Enter SSID here
const char* password = "BingoBongo567";  //Enter Password here

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
int resetDelay = 20000;
int anz = 3;
int kp = 0;

enum Gamemode {
  NORMAL,
  START_STOP
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
  }
  else {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("Access Point created");
  Serial.println(WiFi.localIP());
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
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/change_gamemode", HTTP_POST, [](AsyncWebServerRequest *request){
    gamemode = NORMAL;
    request->send(200);
  });

  server.serveStatic("/", SPIFFS, "/");

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
      tone(buzzer, 1300, 50);
      delay(buzzerDelay);
      digitalWrite(gameLEDS[i], LOW);
      tone(buzzer, 1300, 50);
      delay(buzzerDelay);
      digitalWrite(gameLEDS[i], HIGH);
      tone(buzzer, 1300, 50);
      delay(buzzerDelay);
      digitalWrite(gameLEDS[i], LOW);
      tone(buzzer, 1300, 50);
      delay(buzzerDelay);
      digitalWrite(gameLEDS[i], HIGH);
      tone(buzzer, 1300, 50);

      if (!clientConnected) {
        Serial.print("gebuzzert: LEDpin: ");
        Serial.print(gameLEDS[i]);
        Serial.print(" Buzzerpin: ");
        Serial.println(gameBTNS[i]);

        kp = 1;
        while (digitalRead(schalter) == HIGH && kp < 300) {
          delay(50); 
          kp++;
        }
        
        reset();
        Serial.println("Zeit-Reset!");
        delay(500);
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
    reset();
    Serial.println("manueller Reset!");
    resetTriggered = false;
  }
}