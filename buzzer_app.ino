#include <WiFi.h>
#include <WebServer.h>

/* Put your SSID & Password */
const char* ssid = "BuzzerBongoTill";  // Enter SSID here
const char* password = "BingoBongo567";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

//3 LEDS
int gameLEDS[] = { 14,27,26 };
//3 BUTTONS
int gameBTNS[] = { 15,5,4 };
//PIEZO BUZZER 
int buzzer = 13;
//ALL DELAYS
int buzzerDelay = 100;
int blinkDelay = 500;
int resetDelay = 6000;

enum Gamemode {
  NORMAL,
  START_STOP
};


void setup() {
  Serial.begin(115200);
  for (int i; i < 3; i++) {
    pinMode(gameLEDS[i], OUTPUT);
  }
  //This loops and sets up the Button pins to be INPUT_PULLUP in an array. Ground is HIGH so signal goes LOW on depress.
  for (int i; i < 3; i++) {
    pinMode(gameBTNS[i], INPUT_PULLUP);
  }
  
  enum Gamemode gamemode = NORMAL;
  bool resetTriggered = false;
  bool clientConnected = false;
  bool running = false;
  bool resettet = true;
  float zeit = 0.0;

  unsigned long timeStart = 0; 

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/change_gamemode_normal", handle_change_gamemode_normal);
  server.on("/change_gamemode_start_stop", handle_change_gamemode_start_stop);
  server.on("/reset", handle_reset);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("Server jestachtet");
}

void all_blink_and_buzz() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(gameLEDS[i], HIGH);
  }
  tone(buzzer, 1300, 50);
  delay(blinkDelay);
  for (int i = 0; i < 3; i++) {
    digitalWrite(gameLEDS[i], LOW);
  }
}

void reset() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(gameLEDS[i], LOW);
  }
  resettet = true;
}

void normal_game() {
  for (int i = 0; i < 3; i++) {
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
        Serial.println("Reset!");
        // Auto resets after 5 seconds
        delay(resetDelay); 
        // needs to be manually reset when game show button is depressed
        //for (;;);
        Serial.println(gameLEDS[i]);
        Serial.println(gameBTNS[i]);
        reset();
      }
    }
  }
}

void start_stop_game() {
  for (int i = 0; i < 3; i++) {
    //Press the button to intiate which button chimes in
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
        zeit = elapsedInSec;
      }     
    }
  }
}

void loop() {
  server.handleClient();
  if (gamemode == NORMAL) {
    normal_game();
  }
  else if (gamemode == START_STOP) {
    resettet = true;
    start_stop_game();
  }

  if (resetTriggered) {
    reset();
    resetTriggered = false;
  }
}

void handle_OnConnect() {
  if (!clientConnected) {
    clientConnected = true;
    Serial.println("Tilli connected");
  }
  server.send(200, "text/html", SendHTML());
}

void handle_change_gamemode_normal() {
  gamemode = NORMAL;
  server.send(200, "text/html", SendHTML());
}

void handle_change_gamemode_start_stop() {
  gamemode = START_STOP;
  server.send(200, "text/html", SendHTML());
}

void handle_reset() {
  resetTriggered = true;
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  ptr += "<meta http-equiv=\“refresh\“ content=\“1\“>\n";
  ptr += "<title>Buzzerjeraet Jakob und Till</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Buzzer pour la Gang</h1>\n";
  ptr += "<h3>by Bongobrain und Till dem Boss</h3>\n";
  ptr += "<h4>Vergangene Zeit: ";
  ptr += String(zeit);
  ptr += "s</h4>";

  switch(gamemode) {
    case NORMAL: ptr += "<p>Gamemode</p><a class=\"button button-change_gamemode_start_stop\" href=\"/change_gamemode_start_stop\">Start/Stop</a>\n"; break;
    case START_STOP: ptr += "<p>Gamemode</p><a class=\"button button-change_gamemode_normal\" href=\"/change_gamemode_normal\">Normal</a>\n"; break;
  }

  ptr += "<a class=\"button button-reset\" href=\"/reset\">Reset</a>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
