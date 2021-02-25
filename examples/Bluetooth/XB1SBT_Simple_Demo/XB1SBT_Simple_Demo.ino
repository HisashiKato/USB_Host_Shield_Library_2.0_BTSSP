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

/* You can create the instance of the XB1SBTS class in two ways */
// This will start an inquiry and then pair with the XB1S controller - you only have to do this once
// Press the Xbox button to turn on the power, 
// and You will need to hold down the Sync button at the same time,
// the Xbox button will then start to blink rapidly indicating that it is in pairing mode
XB1SBTS Xb1s(&Btdssp, PAIR);

// After that you can simply create the instance like so
//XB1SBTS Xb1s(&Btdssp);


void setup() {

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

  if (Xb1s.connected()) {
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


    if (Xb1s.buttonClick(A)) {
      Serial.print(F("\r\nA"));
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
  }
}
