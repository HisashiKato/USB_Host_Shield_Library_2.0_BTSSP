/*
 Example sketch for the Xbox One S Controller Bluetooth library
 by HisashiKato
 Web : http://kato-h.cocolog-nifty.com/khweblog/

 Based on the following. 

 Example sketch for the PS4 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com

 Caution:
 Be careful when using the Rumble function.
 It will be crashed depending on how it is used. 

 
How to use Rumble Demo.
 1.Press D-pad or X to set enable rumble motors.
  (Press X is set to enable all rumble motors)
 2.Press LB to rumble enabled motors.
 3.Press LT or RT to rumble Leftside or Rightside motors.

 2b.Press A or B for choice rumble pulse demo pattern.
 3b.Press RB to start rumble demo pattern.

 If the rumble malfunctioned, press Y to stop rumble and init.   
 If it still doesn't stop, Please reset or remove battery cell. 

*/

#include <XB1SBTR.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the XB1SBTR class in two ways */
// This will start an inquiry and then pair with the XB1S controller - you only have to do this once
// Press the Xbox button to turn on the power, 
// and You will need to hold down the Sync button at the same time,
// the Xbox button will then start to blink rapidly indicating that it is in pairing mode
XB1SBTR Xb1s(&Btdssp, PAIR);

// After that you can simply create the instance like so
//XB1SBTR Xb1s(&Btdssp);


void setup() {

  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nXb1s Bluetooth Library Started"));
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

    int magLeft = map(Xb1s.triggerData(LT), 0, 1023, 0, 100);
    int magRight = map(Xb1s.triggerData(RT), 0, 1023, 0, 100);
    if( ! Xb1s.buttonData(LB)){  // If the rumble data transmit is not restrained,it will be crashed.
      Xb1s.rumble(magLeft, magRight, magLeft, magRight);
    }
/*
      Serial.print(F("\r\nmagLeft: "));
      Serial.print(magLeft);
      Serial.print(F("\tmagRight: "));
      Serial.print(magRight);
*/

    if (Xb1s.dpadClick(DPAD_UP)) {
      Serial.print(F("\r\nDPAD_UP"));
      Xb1s.rumbleEnable(RUMBLE_TRIGGER_ONLY);
      Xb1s.rumblePulseEnable(RUMBLE_TRIGGER_ONLY);
      Serial.print(F("\r\nRUMBLE_TRIGGER_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_UP_RIGHT)) {
      Serial.print(F("\r\nDPAD_UP_RIGHT"));
      Xb1s.rumbleEnable(RUMBLE_RIGHT_TRIGGER);
      Xb1s.rumblePulseEnable(RUMBLE_RIGHT_TRIGGER);
      Serial.print(F("\r\nRUMBLE_RIGHT_TRIGGER"));
    }
    if (Xb1s.dpadClick(DPAD_RIGHT)) {
      Serial.print(F("\r\nDPAD_RIGHT"));
      Xb1s.rumbleEnable(RUMBLE_RIGHT_ONLY);
      Xb1s.rumblePulseEnable(RUMBLE_RIGHT_ONLY);
      Serial.print(F("\r\nRUMBLE_RIGHT_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_RIGHT_DOWN)) {
      Serial.print(F("\r\nDPAD_RIGHT_DOWN"));
      Xb1s.rumbleEnable(RUMBLE_RIGHT_WEAK);
      Xb1s.rumblePulseEnable(RUMBLE_RIGHT_WEAK);
      Serial.print(F("\r\nRUMBLE_RIGHT_WEAK"));
    }
    if (Xb1s.dpadClick(DPAD_DOWN)) {
      Serial.print(F("\r\nDPAD_DOWN"));
      Xb1s.rumbleEnable(RUMBLE_MAIN_ONLY);
      Xb1s.rumblePulseEnable(RUMBLE_MAIN_ONLY);
      Serial.print(F("\r\nRUMBLE_MAIN_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_DOWN_LEFT)) {
      Serial.print(F("\r\nDPAD_DOWN_LEFT"));
      Xb1s.rumbleEnable(RUMBLE_LEFT_STRONG);
      Xb1s.rumblePulseEnable(RUMBLE_LEFT_STRONG);
      Serial.print(F("\r\nRUMBLE_LEFT_STRONG"));
    }
    if (Xb1s.dpadClick(DPAD_LEFT)) {
      Serial.print(F("\r\nDPAD_LEFT"));
      Xb1s.rumbleEnable(RUMBLE_LEFT_ONLY);
      Xb1s.rumblePulseEnable(RUMBLE_LEFT_ONLY);
      Serial.print(F("\r\nRUMBLE_LEFT_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_LEFT_UP)) {
      Serial.print(F("\r\nDPAD_LEFT_UP"));
      Xb1s.rumbleEnable(RUMBLE_LEFT_TRIGGER);
      Xb1s.rumblePulseEnable(RUMBLE_LEFT_TRIGGER);
      Serial.print(F("\r\nRUMBLE_LEFT_TRIGGER"));
    }     


    if (Xb1s.buttonClick(A)) {
      Serial.print(F("\r\nA"));
      Serial.print(F("\r\nSetRumblePulseDemoPattern A"));
      Xb1s.rumblePulseMagSet(50, 100, 50, 100);
      Xb1s.rumblePulseLoopSet(10, 5, 5);
    }
    if (Xb1s.buttonClick(B)) {
      Serial.print(F("\r\nB"));
      Serial.print(F("\r\nSetRumblePulseDemoPattern B"));
      Xb1s.rumblePulseMagSet(100, 100, 100, 100);
      Xb1s.rumblePulseLoopSet(50, 50, 2);
    }
    if (Xb1s.buttonClick(X)) {
      Serial.print(F("\r\nX"));
      Xb1s.rumbleEnable(RUMBLE_ALL);
      Xb1s.rumblePulseEnable(RUMBLE_ALL);
      Serial.print(F("\r\nRUMBLE_ALL"));
    }
    if (Xb1s.buttonClick(Y)) {
      Serial.print(F("\r\nY"));
      Xb1s.rumbleStopAll();
      Serial.print(F("\r\nrumbleStopAll"));
      delay(100);
      Xb1s.rumbleInit();
      Serial.print(F("\r\nrumbleInit"));
    }

//    if (Xb1s.buttonClick(LB))
    if (Xb1s.buttonData(LB)){
      Serial.print(F("\r\nLB"));
    }
    int magAll = (Xb1s.buttonData(LB) * 100);
    if((magLeft == 0) && (magRight == 0)){ // If the rumble data transmit is not restrained,it will be crashed.
      Xb1s.rumble(magAll, magAll, magAll, magAll);
    }
    if (Xb1s.buttonClick(RB)) {
      Serial.print(F("\r\nRB"));
      Xb1s.rumblePulseStart();
      Serial.print(F("\r\nrumblePulseStart"));
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
      Xb1s.rumbleTest(0b0110,50,50,50,50,100,200,2);
      Serial.print(F("\r\nrumbleTest"));
      Serial.print(F("\r\n0b0110,50,50,50,50,100,200,2"));
      Serial.print(F("\r\nRumbleMotorEnable:0b0110, Mag:50, VibrationTime:100x10ms, StartWaiting:200x10ms, LoopCount:2(Vibrate 3 times)"));
    }
    if (Xb1s.buttonClick(XBOX)) {
      Serial.print(F("\r\nXBOX"));
    }
  }
}
