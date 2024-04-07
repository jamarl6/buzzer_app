#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

const char* ssid = "GangBuzzer";
const char* password = "TillistderBoss";
unsigned long startTime = 0;

bool running = false;

WiFiServer server(80);

//3 LEDS
int gameLEDS[] = { 14,25,26 };
//3 BUTTONS
int gameBTNS[] = { 15,5,4 };
//PIEZO BUZZER 
int buzzer = 13;
//ALL DELAYS
int buzzerDelay = 100;
int resetDelay = 12000;


void setup() {

  //Serial begin allows the serial port to read and monitor functions
  Serial.begin(115200);
  //This loops and sets up the LED pins to be output in an array
  for (int i; i < 3; i++) {
    pinMode(gameLEDS[i], OUTPUT);
  }
  //This loops and sets up the Button pins to be INPUT_PULLUP in an array. Ground is HIGH so signal goes LOW on depress.
  for (int i; i < 3; i++) {
    pinMode(gameBTNS[i], INPUT_PULLUP);
  }

  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server jestachtet");
}



void loop() {

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            //THIS WILL ALWAYS LOOP UNLESS YOU PUT A FOR LOOP STOPPER @ THE END
            // This loops thru the buzzer buttons to define which is pressed

            for (int i; i < 3; i++) {
              //Press the button to intiate which button chimes in
              if (digitalRead(gameBTNS[i]) == LOW) {

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

                if (!running) {
                  // Starte die Stoppuhr
                  startTime = millis();
                  running = true;
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-Type: text/html");
                  client.println();
                  client.println("<p>gameLEDS[i]</p>");
                  client.println("</body></html>");
                  client.println();
                  client.stop();
                }
                else {
                  // Stoppe die Stoppuhr und zeige die Zeit auf dem Webserver an
                  unsigned long elapsedTime = millis() - startTime;
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-Type: text/html");
                  client.println();
                  client.print("<html><body><h1>Elapsed Time: ");
                  client.print(elapsedTime);
                  client.println(" milliseconds</h1></body></html>");
                  // Zusätzliche Informationen hinzufügen
                  client.println("<p>gameLEDS[i]</p>");
                  client.println("</body></html>");
                  client.println();
                  client.stop();
                }
              }
              else {
                digitalWrite(gameLEDS[i], LOW);
              }
            }
            // Auto resets after 5 seconds
            delay(resetDelay);
            // needs to be manually reset when game show button is depressed
            //for (;;);
            Serial.println(gameLEDS[i]);
            Serial.println(gameBTNS[i]);
          }
        }
        else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
        }
        else {    // if you got a newline, then clear currentLine:
            currentLine = "";
        }
      }
    }
  }
}
