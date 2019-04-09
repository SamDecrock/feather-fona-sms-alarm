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
char settingsUrl[] = "http://19fd92e2.ngrok.io/api/m2m/instances/instance-0-smsalarm/settings";


bool callIsReady = false;
bool smsIsReady = false;
bool connectedToNetwork = false;

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

  pinMode(circuitPin, INPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }



  digitalWrite(green, 0);
  digitalWrite(red, 0);
}

void loop() {
  // while there is data on the Fona serial, read it and spit it out:
  // while (fona.available()) {
  //  Serial.write(fona.read());
  // }


  if ((millis() - lastNetworkCheckTime) > 10000) {
    lastNetworkCheckTime = millis();
    checkNetwork();

    if(connectedToNetwork) {
      checkGPRS();
    }

  }


  readCircuit();


}

void checkNetwork() {
  // checks connection to network:
  uint16_t networkStatus = fona.getNetworkStatus();
  if((networkStatus == 1 || networkStatus == 5) && !connectedToNetwork) {
    connectedToNetwork = true;
    onConnectedToNetwork();
  }

  if(networkStatus != 1 && networkStatus != 5 && connectedToNetwork) {
    connectedToNetwork = false;
    onDisconnectedFromNetwork();
  }
}

void onConnectedToNetwork() {
  fona.setGPRSNetworkSettings(F("web.be"), F("web"), F("web"));
  fona.enableGPRS(true);
  Serial.println("connected to network!");
  digitalWrite(green, 1);
}

void onDisconnectedFromNetwork() {
  Serial.println("disconnected from network!");
  digitalWrite(green, 0);
}

void checkGPRS() {
  uint8_t gprsState = fona.GPRSstate();
  if(gprsState == 0) {
    Serial.println("enabling gprs");
    fona.enableGPRS(false);
    fona.enableGPRS(true);
  }else if (gprsState == 1){
    Serial.println("gprs attached");
    digitalWrite(green, 1);
  }
}

void getSettings() {
  uint16_t statuscode;
  int16_t length;

  if (!fona.HTTP_GET_start(settingsUrl, &statuscode, (uint16_t *)&length)) {
    Serial.println("Failed to get settings");
    return;
  }

  while (length > 0) {
    while (fona.available()) {
      char c = fona.read();
      Serial.write(c);
      length--;
    }
  }
  Serial.println(F("HTTP GET end"));
  fona.HTTP_GET_end();
}

void readCircuit() {
  // read the state of the switch into a local variable:
  int currentCircuitState = digitalRead(circuitPin);

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
