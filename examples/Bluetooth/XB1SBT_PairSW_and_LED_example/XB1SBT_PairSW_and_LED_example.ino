/*
 Example sketch for the Xbox One S Controller Bluetooth library
 by HisashiKato
 Web : http://kato-h.cocolog-nifty.com/khweblog/

 Based on the following. 

 Example sketch for the PS4 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
*/

#include <XB1SBTS.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so


/* This is an example sketch aimed at using the pairing button and showing the connection status with a LED. */

// When the Arduino turn on or reset, it will wait for a connection and the LED will blink.
// While waiting for connection, if the pairing button press and hold for 3 seconds or longer, 
// it will switch to waiting for pairing and the LED will blink quickly.
// Press the Xbox button to turn on the controller's power,
// and hold down the Sync button at the same time,
// the Xbox button will then start to blink quickly indicating that it is in pairing mode.
// If pairing is performed and successful, the Arduino's LED will turn off, and Xbox button will light on.
// After successful pairing, simply turn on power each other and they will be connected.
XB1SBTS Xb1s(&Btdssp);

// LED Status
#define LED_ON  1
#define LED_OFF 0
#define BLINK_SLOW   1000
#define BLINK_MEDIUM 500
#define BLINK_FAST   200

// Connection status
#define BT_DISCONNECT     0
#define BT_CONNECT        1
#define BT_PAIRING_WAIT   2


const int extBtnPin = 2; // the number of the External Button pin. In this example, the button (switch) is connected to pin2 and ground.
const int extLedPin = 3; // the number of the External LED pin. In this example, the LED is connected to pin3 and ground via resistor. 

int ledStatus = BLINK_SLOW;

bool extLed = LOW;
bool oldExtLed = LOW;

unsigned long currentMillisLED; 
unsigned long previousMillisLED = 0;

bool extButton = HIGH;
unsigned long currentMillisButton; 
unsigned long previousMillisButton = 0;

int connectionStatus;
int oldConnectionStatus = 0;

bool turnToPairingMode = 0;
bool alreadyConnected = 0;


void setup() {
//---------------
    pinMode(extBtnPin, INPUT_PULLUP);
    pinMode(extLedPin, OUTPUT);

    connectionStatus = BT_DISCONNECT;
    setLedstatus(connectionStatus);
//---------------

  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nXB1S Bluetooth Library Started"));
}

void loop() {

  Usb.Task();
//---------------
    pairingButtonCheck();
    pairModeSwitching();
    blinkLed(ledStatus);
//---------------

  if (Xb1s.connected()) {
    alreadyConnected = 1;
    connectionStatus = BT_CONNECT;
    setLedstatus(connectionStatus);

    if (Xb1s.stickData(STICK_LX) > (32767+7680) || Xb1s.stickData(STICK_LX) < (32767-7680) || Xb1s.stickData(STICK_LY) > (32767+7680) || Xb1s.stickData(STICK_LY) < (32767-7680)) {
      Serial.print(F("\r\nStick_LX: "));
      Serial.print(Xb1s.stickData(STICK_LX));
      Serial.print(F("\tStick_LY: "));
      Serial.print(Xb1s.stickData(STICK_LY));
    }
    if (Xb1s.stickData(STICK_RX) > (32767+7680) || Xb1s.stickData(STICK_RX) < (32767-7680) || Xb1s.stickData(STICK_RY) > (32767+7680) || Xb1s.stickData(STICK_RY) < (32767-7680)) {
      Serial.print(F("\r\nStick_RX: "));
      Serial.print(Xb1s.stickData(STICK_RX));
      Serial.print(F("\tStick_RY: "));
      Serial.print(Xb1s.stickData(STICK_RY));
    }

    if (Xb1s.triggerData(LT) > 0) {
      Serial.print(F("\r\nTrigger_LT: "));
      Serial.print(Xb1s.triggerData(LT));
    }
    if (Xb1s.triggerData(RT) > 0) {
      Serial.print(F("\r\nTrigger_RT: "));
      Serial.print(Xb1s.triggerData(RT));
    }


    if (Xb1s.dpadClick(DPAD_UP)) {
      Serial.print(F("\r\nDPAD_UP"));
    }
    if (Xb1s.dpadClick(DPAD_UP_RIGHT)) {
      Serial.print(F("\r\nDPAD_UP_RIGHT"));
    }
    if (Xb1s.dpadClick(DPAD_RIGHT)) {
      Serial.print(F("\r\nDPAD_RIGHT"));
    }
    if (Xb1s.dpadClick(DPAD_RIGHT_DOWN)) {
      Serial.print(F("\r\nDPAD_RIGHT_DOWN"));
    }
    if (Xb1s.dpadClick(DPAD_DOWN)) {
      Serial.print(F("\r\nDPAD_DOWN"));
    }
    if (Xb1s.dpadClick(DPAD_DOWN_LEFT)) {
      Serial.print(F("\r\nDPAD_DOWN_LEFT"));
    }
    if (Xb1s.dpadClick(DPAD_LEFT)) {
      Serial.print(F("\r\nDPAD_LEFT"));
    }
    if (Xb1s.dpadClick(DPAD_LEFT_UP)) {
      Serial.print(F("\r\nDPAD_LEFT_UP"));
    }     


//    if (Xb1s.buttonClick(A)) {
    if (Xb1s.buttonData(A)) {
      Serial.print(F("\r\nA"));
      digitalWrite(extLedPin, HIGH);
    } else {
      digitalWrite(extLedPin, LOW);
    }
    if (Xb1s.buttonClick(B)) {
      Serial.print(F("\r\nB"));
    }
    if (Xb1s.buttonClick(X)) {
      Serial.print(F("\r\nX"));
    }
    if (Xb1s.buttonClick(Y)) {
      Serial.print(F("\r\nY"));
    }

    if (Xb1s.buttonClick(LB))
      Serial.print(F("\r\nLB"));
    if (Xb1s.buttonClick(RB)) {
      Serial.print(F("\r\nRB"));
    }
    if (Xb1s.buttonClick(RS))
      Serial.print(F("\r\nRS"));
    if (Xb1s.buttonClick(LS))
      Serial.print(F("\r\nLS"));


    if (Xb1s.buttonClick(VIEW)) {
      Serial.print(F("\r\nVIEW"));
      Serial.print(F("\r\nBatteryStatus:"));
      Serial.print(Xb1s.batteryStatus(), BIN);
      Serial.print(F("\r\nBatteryLevel:"));
      if(Xb1s.batteryLevel() == 3) Serial.print(F("Full"));
      else if(Xb1s.batteryLevel() == 2) Serial.print(F("High"));
      else if(Xb1s.batteryLevel() == 1) Serial.print(F("Normal"));
      else {
        Serial.print(F("Low:"));
        Serial.print(Xb1s.batteryLevel(), BIN);
      }
    }
    if (Xb1s.buttonClick(MENU)) {
      Serial.print(F("\r\nMENU"));
    }
    if (Xb1s.buttonClick(XBOX)) {
      Serial.print(F("\r\nXBOX"));
    }
  } else if (connectionStatus != BT_PAIRING_WAIT) {
    connectionStatus = BT_DISCONNECT;
    setLedstatus(connectionStatus);
  }
}

//----------------------------

void pairingButtonCheck(void) {
    currentMillisButton = millis();

    if (digitalRead(extBtnPin) == LOW){
        if (currentMillisButton - previousMillisButton >= 3000){
            turnToPairingMode = 1;
            previousMillisButton = currentMillisButton;
        }
    } else {
        previousMillisButton = currentMillisButton;
    }
}

void pairModeSwitching(void) {
    if (turnToPairingMode == 1) {
        if (alreadyConnected == 0) {
            connectionStatus = BT_PAIRING_WAIT;
            setLedstatus(connectionStatus);
            Serial.print(F("\r\nTurn To Pairing Mode."));
            Xb1s.pair();
        }
        turnToPairingMode = 0;
    }
}

void setLedstatus(int status) {
    switch(status) {
        case BT_DISCONNECT:
            ledStatus = BLINK_SLOW;
            break;
        case BT_CONNECT:
            ledStatus = LED_OFF;
            break;
        case BT_PAIRING_WAIT:
            ledStatus = BLINK_FAST;
            break;
        default:
            ledStatus = LED_OFF;
    }
}


void blinkLed(int status) {
    currentMillisLED = millis();
    if (status == LED_OFF) {
        extLed = LOW;
    } else if (status == LED_ON) {
        extLed = HIGH;
    } else if (currentMillisLED - previousMillisLED >= (unsigned long)status) {
        previousMillisLED = currentMillisLED;
        extLed = !extLed;
    }
    if (oldExtLed != extLed) {
        digitalWrite(extLedPin, extLed);
    }
    oldExtLed = extLed;
}
//----------------------------
