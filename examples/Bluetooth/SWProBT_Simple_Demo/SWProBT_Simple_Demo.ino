/*
 Example sketch for the Switch Pro Controller Bluetooth library
 by HisashiKato
 Web : http://kato-h.cocolog-nifty.com/khweblog/

 Based on the following. 

 Example sketch for the PS4 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
*/

#include <SWProBT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the SWProBT class in two ways */
// This will start an inquiry and then pair with the Switch Pro controller - you only have to do this once
// You will need to press and hold the Sync button on the top of the Switch Pro Controller for a few seconds
// until the indicator lights start flashing that it is in pairing mode.
SWProBT SWPro(&Btdssp, PAIR);

// After that you can simply create the instance like so.
//SWProBT SWPro(&Btdssp);


void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nSWPro Bluetooth Library Started"));
}

void loop() {
  Usb.Task();

  if (SWPro.connected()) {
    if (SWPro.stickData(STICK_LX) > (32767+7680) || SWPro.stickData(STICK_LX) < (32767-7680) || SWPro.stickData(STICK_LY) > (32767+7680) || SWPro.stickData(STICK_LY) < (32767-7680)) {
      Serial.print(F("\r\nStick_LX: "));
      Serial.print(SWPro.stickData(STICK_LX));
      Serial.print(F("\tStick_LY: "));
      Serial.print(SWPro.stickData(STICK_LY));
    }
    if (SWPro.stickData(STICK_RX) > (32767+7680) || SWPro.stickData(STICK_RX) < (32767-7680) || SWPro.stickData(STICK_RY) > (32767+7680) || SWPro.stickData(STICK_RY) < (32767-7680)) {
      Serial.print(F("\r\nStick_RX: "));
      Serial.print(SWPro.stickData(STICK_RX));
      Serial.print(F("\tStick_RY: "));
      Serial.print(SWPro.stickData(STICK_RY));
    }


    if (SWPro.dpadClick(DPAD_UP))
      Serial.print(F("\r\nDPAD_UP"));
    if (SWPro.dpadClick(DPAD_UP_RIGHT))
      Serial.print(F("\r\nDPAD_UP_RIGHT"));
    if (SWPro.dpadClick(DPAD_RIGHT))
      Serial.print(F("\r\nDPAD_RIGHT"));
    if (SWPro.dpadClick(DPAD_RIGHT_DOWN))
      Serial.print(F("\r\nDPAD_RIGHT_DOWN"));
    if (SWPro.dpadClick(DPAD_DOWN))
      Serial.print(F("\r\nDPAD_DOWN"));
    if (SWPro.dpadClick(DPAD_DOWN_LEFT))
      Serial.print(F("\r\nDPAD_DOWN_LEFT"));
    if (SWPro.dpadClick(DPAD_LEFT))
      Serial.print(F("\r\nDPAD_LEFT"));
    if (SWPro.dpadClick(DPAD_LEFT_UP))
      Serial.print(F("\r\nDPAD_LEFT_UP"));


    if (SWPro.buttonClick(A)) {
      Serial.print(F("\r\nA"));
    }
    if (SWPro.buttonClick(B)) {
      Serial.print(F("\r\nB"));
    }
    if (SWPro.buttonClick(X)) {
      Serial.print(F("\r\nX"));
    }
    if (SWPro.buttonClick(Y)) {
      Serial.print(F("\r\nY"));
    }

    if (SWPro.buttonClick(LB))
      Serial.print(F("\r\nLB"));
    if (SWPro.buttonClick(RB))
      Serial.print(F("\r\nRB"));

    if (SWPro.buttonClick(ZL))
      Serial.print(F("\r\nZL"));
    if (SWPro.buttonData(ZL)) {
      SWPro.simpleRumbleL(1);
    }else{
      SWPro.simpleRumbleL(0);
    } 

    if (SWPro.buttonClick(ZR))
      Serial.print(F("\r\nZR"));
    if (SWPro.buttonData(ZR)) {
      SWPro.simpleRumbleR(1);
    }else{
      SWPro.simpleRumbleR(0);
    } 


    if (SWPro.buttonClick(MINUS))
      Serial.print(F("\r\nMINUS"));
    if (SWPro.buttonClick(PLUS))
      Serial.print(F("\r\nPLUS"));

    if (SWPro.buttonClick(LS))
      Serial.print(F("\r\nLS"));
    if (SWPro.buttonClick(RS))
      Serial.print(F("\r\nRS"));


    if (SWPro.buttonClick(HOME)) {
      Serial.print(F("\r\nHOME"));
    }
    if (SWPro.buttonClick(CAPTURE)) {
      Serial.print(F("\r\nCAPTURE"));
    }

  }
}
