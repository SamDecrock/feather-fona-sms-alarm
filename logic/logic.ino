#include "Adafruit_FONA.h"

#define FONA_RX 9
#define FONA_TX 8
#define FONA_RST 4

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

const int buttonPin = A0;    // the number of the pushbutton pin
const int green = 12;      // the number of the LED pin
const int red = 11;

bool fonaConnected = false;

// Variables will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);
  Serial.println("hello");

  pinMode(buttonPin, INPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);

  // set initial LED state
  digitalWrite(green, 0);

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  Serial.println(F("FONA is OK"));
  
  fonaConnected = true;
  digitalWrite(green, 1);
  digitalWrite(red, 0);
}

void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  digitalWrite(red, !reading);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      Serial.println(buttonState);

      

      if(fonaConnected) {
        digitalWrite(green, buttonState);

        if(buttonState == 0) {
          sendSms();
        }
      }
    }
  }

  // set the LED:


  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}

void sendSms() {

  // send an SMS!
  char sendto[] = "0485XXXXXX";
  char message[] = "Alarm";
  
  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Message failed"));
  } else {
    Serial.println(F("Message Sent!"));
    digitalWrite(green, 0);
    delay(100);
    digitalWrite(green, 1);
    delay(100);
    digitalWrite(green, 0);
    delay(100);
    digitalWrite(green, 1);
    delay(100);
    digitalWrite(green, 0);
    delay(100);
    digitalWrite(green, 1);
    delay(100);
    digitalWrite(green, 0);
  }

  digitalWrite(green, buttonState);
}
