#include "Adafruit_FONA.h"


#define FONA_RX 9
#define FONA_TX 8
#define FONA_RST 4

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

const int circuitPin = A0;
const int green = 12;
const int red = 11;

bool connectedToNetwork = false;
bool networkDisconnectCounter = 0;

// Variables will change:
int circuitState;           // the current circuit state
int lastCircuitState = 0;   // the previous circuit state

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

unsigned long lastNetworkCheckTime = 0;
unsigned long lastSendSmsTime = 0;

void setup() {
  Serial.begin(115200);
  //while (!Serial) {}
  Serial.println("just started");


  // configure pins:
  pinMode(circuitPin, INPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);

  // turn off LEDs:
  digitalWrite(green, 0);
  digitalWrite(red, 0);

  // connect to FONA800 modem:
  fonaSerial->begin(4800);

  // initialize modem:
  bool modemInitSuccess = fona.begin(*fonaSerial);

  if (!modemInitSuccess) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // halt!
  }
}

void loop() {
  // while there is data on the Fona serial, read it and spit it out:
  // while (fona.available()) {
  //  Serial.write(fona.read());
  // }


  if((millis() - lastNetworkCheckTime) > 10000) {
    lastNetworkCheckTime = millis();
    checkNetwork();
  }


  readCircuit();
}

// happens every 10 seconds:
void checkNetwork() {

  uint16_t networkStatus = fona.getNetworkStatus();
  if((networkStatus == 1 || networkStatus == 5)) {
    connectedToNetwork = true;
    networkDisconnectCounter = 0;
  }

  if(networkStatus != 1 && networkStatus != 5) {
    connectedToNetwork = false;
    networkDisconnectCounter++;
  }

  // update green LED according to network state:
  digitalWrite(green, connectedToNetwork);


  // check if network connection is down too long:
  if(networkDisconnectCounter >= 6) { // after a minute of disconnected
    Serial.println(F("Modem disonnected for 1 minute... will re-init modem."));
    // re-initialize modem:
    bool modemInitSuccess = fona.begin(*fonaSerial);
    networkDisconnectCounter = 0;

    if(!modemInitSuccess) {
      Serial.println(F("Re-init of modem failed"));
    }
  }
}


void readCircuit() {
  // read the state of the switch into a local variable:
  int currentCircuitState = digitalRead(circuitPin);

  // turn on red LED when circuit is open:
  digitalWrite(red, !currentCircuitState);

  // check to see if circuit broke ar not
  // (i.e. the input went from LOW to HIGH), and you've waited
  // long enough since the change to ignore any noise:

  // If the circuit state changed, due to noise or actual brake:
  if (currentCircuitState != lastCircuitState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the currentCircuitState is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the circuit state has changed:
    if (currentCircuitState != circuitState) {
      circuitState = currentCircuitState;

      Serial.print("circuit state: ");
      Serial.println(circuitState);

      if(circuitState == 0) {
        sendSms();
      }

    }
  }

  // save the currentCircuitState.  Next time through the loop,
  // it'll be the lastCircuitState:
  lastCircuitState = currentCircuitState;


  // keep sending sms every minute if circuit is broken
  if ((millis() - lastSendSmsTime) > 60000) {
    lastSendSmsTime = millis();

    if(circuitState == 0) {
      sendSms();
    }
  }
}

void sendSms() {
  if(!connectedToNetwork) return;

  // send an SMS!
  char sendto[] = "0485xxxxxx";
  char message[] = "Alarm";

  Serial.println("Sending Alarm sms");

  if (!fona.sendSMS(sendto, message)) {
    Serial.println("Message failed");
  } else {
    Serial.println("Message Sent!");
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
  }
}
