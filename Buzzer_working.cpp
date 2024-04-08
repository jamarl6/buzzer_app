// Game show Buzzer and lights for quick draw gaming.
//CREDIT TO: MIKE MERRYMAN

//3 LEDS
int gameLEDS[] = {14,25,26};
//3 BUTTONS
//int gameBTNS[] = {15,5,4};
int gameBTNS[] = {32,35,34};
//PIEZO BUZZER 
int buzzer = 13;
//ALL DELAYS
int buzzerDelay= 100;
int resetDelay= 12000;

unsigned long startTime = 0;
unsigned long elaspedTime;

bool running = false;


void setup() {

//Serial begin allows the serial port to read and monitor functions
      Serial.begin(115200);
//This loops and sets up the LED pins to be output in an array
      for (int i; i<3; i++){
      pinMode(gameLEDS[i], OUTPUT);
    }
//This loops and sets up the Button pins to be INPUT_PULLUP in an array. Ground is HIGH so signal goes LOW on depress.
      for (int i; i<3; i++){
      pinMode(gameBTNS[i], INPUT_PULLUP);
    }
    
    }

void loop() {
//THIS WILL ALWAYS LOOP UNLESS YOU PUT A FOR LOOP STOPPER @ THE END
// This loops thru the buzzer buttons to define which is pressed

    for (int i; i<3; i++){
      
//Press the button to intiate which button chimes in

    if (digitalRead(gameBTNS[i]) == LOW){

// This is the indicator light and buzzer sound confirmation
     
    digitalWrite(gameLEDS[i], HIGH);

if (!running) {
        // Starte die Stoppuhr
        startTime = millis();
        running = true;
        
      } else {
        // Stoppe die Stoppuhr und zeige die Zeit auf dem Webserver an
        unsigned long elapsedTime = millis() - startTime;
          Serial.println("Vergangene Zeit:");
          Serial.print(elapsedTime/1000.0);
          Serial.println(" Sekunden");
          // Zusätzliche Informationen hinzufügen
          Serial.println(gameLEDS[i]);
          
          running = false;
          }


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
// Auto resets after 5 seconds
    delay(resetDelay); 
// needs to be manually reset when game show button is depressed
    //for (;;);
    Serial.println(gameLEDS[i]);
    Serial.println(gameBTNS[i]);
    
    }else{
      
     digitalWrite(gameLEDS[i], LOW);
         
    }
  }
}
