/*
 *
 * Pomodoro Timer v2
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * If we meet some day, and you think this stuff is worth it, you can buy me a beer in return.
 * - 8/8/2014, Rogier van den Berg, http://www.rogiervandenberg.nl (Regular website)
 * http://blog.rogiervandenberg.nl/ (Arduino / Nerd website ;)  )
 * @RogiervdBerg --> Twitter
 * ----------------------------------------------------------------------------

 This sketch counts down to zero, starting at one of the Countdown options.
 Default is 25 minutes, to facilitate a "Pomodoro Session", see http://www.pomodorotechnique.com/ for details.
 The button restarts a session, or, when not having a session selects the countdown value.
 
 Features:
 - One button interface for reset counting and selecting different modes
 - LED is on when counting down
 - Sound, beep when switching on, sound effect when done counting
 
 What you need:
 - Arduino (I used an Arduino Uno)
 - 4 digit 7 segment display: http://www.sparkfun.com/products/9483 (Datasheet: http://www.sparkfun.com/datasheets/Components/LED/7-Segment/YSD-439AR6B-35.pdf )
 - A button (for restarting the timer and setting the countdown value)
 - A LED (for showing status)
 - A piezo buzzer, for the "Timer done" sound
 
 Some parts of this code are borrowed from Nathan Seidle, thank you!
 */

const int buttonPin = A2;    // the number of the pushbutton pin
const int ledPin = 13;      // the number of the LED pin
const int speakerPin = 3;   // Piezo buzzer PWM

/* Possible start point for counting down */
int countdownOptions[] = {5, 15, 25, 50}; //Possible values to count down from
int countdownSetting = 2;                //Default countdown-value is 25 minutes (Pomodoro Technique)

/* Values to prevent the button from bouncing */
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

/* Values for storing a count down session */
long lastStartTime = 0;    //When did we start
boolean counting = false;  //Are we counting at this moment?
boolean active = true;     //Is the clock active or suspended (active can be "not yet counting, but the button is pressed")

int digit1 = 11; //PWM Display pin 1
int digit2 = 10; //PWM Display pin 2
int digit3 = 9; //PWM Display pin 6
int digit4 = 6; //PWM Display pin 8

int colon = 5; //PWM Display pin 4 //////////////////////////////////////////

//Pin mapping from Arduino to the ATmega DIP28 if you need it
//http://www.arduino.cc/en/Hacking/PinMapping
int segA = A1; //Display pin 14
int segB = A3; //Display pin 16
int segC = 4; //Display pin 13
int segD = 2; //Display pin 3
int segE = A0; //Display pin 5
int segF = 7; //Display pin 11
int segG = 8; //Display pin 15

int segdots = 12; //Display pin 12 /////////////////////

void setup() {                
  //Initializign pins
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH); //Enable Pullup resistor for button
  pinMode(ledPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);

  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);
  pinMode(segdots, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  pinMode(digit4, OUTPUT);
  pinMode(colon, OUTPUT);
  
  //Start Serial for debug
  Serial.begin(9600);
  
  //Beep for "switched on"
 playTone(200, 100);
  
}

void loop() {
  
  // If active for at least 2 seconds, but not yet counting, start counting
 // This enables the user to select another countdown-value, within 2 seconds
 if (!counting && active && (millis() - lastStartTime) > 2000) {
   Serial.print("GO, ");
   Serial.print(countdownOptions[countdownSetting]);
   Serial.println(" minutes");
   counting = true;
   lastStartTime = millis();
 }

 //Make it count
 count();

 //Check for button input
 evaluateButton();  

}

/* This method calculates a new value to show on the display, based on time passed since starting to count */
void count() {
  //If we are not counting, do nothing
  if(counting) {
    //Calculate the value to display in minutes and seconds, instead of milliseconds.
    digitalWrite(ledPin, HIGH); 
    unsigned long now = millis();
    int secondsPassed = (now-lastStartTime)/1000; //The lower the faster the clock counts, 1000 is realtime
    int secondsDisplay = ((countdownOptions[countdownSetting] * 60) - secondsPassed) % 60;
    int minutesDisplay = ((countdownOptions[countdownSetting] * 60) - secondsPassed) / 60;
    
    boolean blinkcolon = ((now-lastStartTime)/500) % 2 == 1; //This calculates whether to switch the semicolon on or off every half a second
    
    if(secondsDisplay < 10) {
     displayNumber((((String)minutesDisplay)+"0"+((String)secondsDisplay)).toInt(), blinkcolon);
    } else {
      displayNumber((((String)minutesDisplay)+((String)secondsDisplay)).toInt(), blinkcolon);
    }
    
    //If we are done, (0:00), stop counting, stop being active, play the end tones and suspend the LCD display
    if(secondsDisplay == 0 && minutesDisplay == 0) {
      counting = false;
      active = false;

      for(int cycle = 0 ; cycle < 8 ; cycle++) { //Play tone and flash the LED
        digitalWrite(ledPin, LOW); 
        playTone(200, 50); 
        digitalWrite(ledPin, HIGH); 
        playTone(200, 50); 
      }
      digitalWrite(ledPin, LOW);   //And turn the LED off
      }
  } else if(active) {
    displayNumber(((  (String)countdownOptions[countdownSetting])+("00")).toInt(), true);
    digitalWrite(ledPin, LOW); 
  } 
}

/* Check if the button is pressed and restart the current session (if active and counting) or select the next count down value */
void evaluateButton() {
  int reading = (digitalRead(buttonPin) * -1); //Read and invert the button status
  
  if (reading != lastButtonState) { //Prevent bounces in button readings
    lastDebounceTime = millis();
  } 
  // Check for bouncing button input
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
       //if active (being handled) but not yet counting down, select the next countdown value
       if (active && !counting) {
          
          if(countdownSetting++ == (sizeof(countdownOptions) / sizeof(int)) - 1) {
            countdownSetting = 0;
          }
        }
        // Stop current counting action (because we were pressing the button) and make sure we are awake (to 
        counting = false;
        active = true; // Wake up!
        
        //Display currently set countdown value
        Serial.println(countdownOptions[countdownSetting]);
        lastStartTime = millis();
      }
    }
  }
  lastButtonState = reading;
}


//Display brightness
//Each digit is on for a certain amount of microseconds
//Then it is off until we have reached a total of 20ms for the function call
//Let's assume each digit is on for 1000us
//If each digit is on for 1ms, there are 4 digits, so the display is off for 16ms.
//That's a ratio of 1ms to 16ms or 6.25% on time (PWM).
//Let's define a variable called brightness that varies from:
//5000 blindingly bright (15.7mA current draw per digit)
//2000 shockingly bright (11.4mA current draw per digit)
//1000 pretty bright (5.9mA)
//500 normal (3mA)
//200 dim but readable (1.4mA)
//50 dim but readable (0.56mA)
//5 dim but readable (0.31mA)
//1 dim but readable in dark (0.28mA)
//This part of the sketch is based on the code from Nathan Seidle, as provided with the display @ https://www.sparkfun.com/products/9483
void displayNumber(int toDisplay, boolean dots) {
#define DISPLAY_BRIGHTNESS  700

#define DIGIT_ON  HIGH
#define DIGIT_OFF  LOW

  long beginTime = millis();

  for(int digit = 4 ; digit > 0 ; digit--) {

    //Turn on a digit for a short amount of time
    if (digit == 1) {
      digitalWrite(digit1, DIGIT_ON);
    } else if (digit == 2) {
      digitalWrite(digit2, DIGIT_ON);
    } else if (digit == 3) {
      digitalWrite(digit3, DIGIT_ON);
      if (dots) {
        digitalWrite(colon, DIGIT_ON);
      }
    } else if (digit == 4) {
      digitalWrite(digit4, DIGIT_ON);
    }

    //123 
    //Turn on the right segments for this digit
    lightNumber(toDisplay % 10);
    toDisplay /= 10;

    delayMicroseconds(DISPLAY_BRIGHTNESS); //Display this digit for a fraction of a second (between 1us and 5000us, 500 is pretty good)

    //Turn off all segments
    lightNumber(10); 

    //Turn off all digits
    digitalWrite(digit1, DIGIT_OFF);
    digitalWrite(digit2, DIGIT_OFF);
    digitalWrite(digit3, DIGIT_OFF);
    digitalWrite(digit4, DIGIT_OFF);
    digitalWrite(colon, DIGIT_OFF);
  }

  while( (millis() - beginTime) < 10) ; //Wait for 20ms to pass before we paint the display again
}

//Given a number, turns on those segments
//If number == 10, then turn off number
void lightNumber(int numberToDisplay) {

#define SEGMENT_ON  LOW
#define SEGMENT_OFF HIGH

digitalWrite(segdots, SEGMENT_ON); //Puntjes aan

  switch (numberToDisplay){

  case 0:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 1:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 2:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_OFF);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 3:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 4:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 5:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 6:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 7:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 8:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 9:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 10:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_OFF);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    digitalWrite(segdots, SEGMENT_OFF); //Puntjes uit
    break;
  }
}

/* Play a tone */
 void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}
