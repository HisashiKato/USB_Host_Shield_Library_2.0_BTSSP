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

 How to use Rumble Pulse Demo.
 1.Press D-pad or X to set for enable rumble motors.
 2.Press A or B for choice rumble pulse demo pattern.
 3.Press RB to start.

 If the rumble malfunctioned, press Y to stop rumble.   
 If it still doesn't stop, Please reset or remove battery cell. 

*/

#include <XB1SBTR.h>
#include <usbhub.h>
#include <EEPROM.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

//#define ENABLE_PAIR

// Remove "//" from the "//#define ENABLE_PAIR" line, enable "#define ENABLE_PAIR",
// and write this sketch to the Arduino to enter the pairing state.
//
// Put the game controller into pairing mode.
// Press the Xbox button to turn on the controller's power,
// and hold down the Sync button at the some time,
// the Xbox button will then start to blink quickly indicating that it is in pairing mode.
// After about 15 seconds, if pairing is performed and successful, Xbox button will light on.
//
// After pairing is complete, add "//" to the "#define ENABLE_PAIR" line to
// make it "//#define ENABLE_PAIR" and write this sketch to the "Arduino" again.
// After that, simply turn on power each other and they will be connected.
//
// (This used a translation site to translate it into English.)


USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so
XB1SBTR Xb1s(&Btdssp);

byte ssp_bdaddr[6];
byte ssp_link_key[16];

#ifdef ENABLE_PAIR
bool stored_ssp_status = false;
#endif


void setup() {

  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.println(F("OSC did not start"));
    while (1); // Halt
  }
  Serial.println(F("XB1S Bluetooth Library Started"));


#ifdef ENABLE_PAIR
  Serial.println(F("Pairing is in progress."));
  Serial.println(F("Please enable pairing on your device."));
#else
  // Read SSP Data fron EEPROM.
  for (int i = 0; i < 6; i++) {
    ssp_bdaddr[i] = EEPROM.read(i);
  }
  for (int i = 0; i < 16; i++) {
    ssp_link_key[i] = EEPROM.read(i + 6);
  }
  // Set SSP Data.
  Xb1s.setConnectAddress(ssp_bdaddr);
  Xb1s.setPairedLinkkey(ssp_link_key);

  Serial.print("setConnectAddress ");
  Serial.print(ssp_bdaddr[0],HEX);
  for(int i = 1 ; i < 6; i++) {
    Serial.print(":");
    Serial.print(ssp_bdaddr[i],HEX);
  }
  Serial.println("");

  Serial.print("setPairedLinkkey ");
  Serial.print(ssp_link_key[0],HEX);
  for(int i = 1 ; i < 16; i++) {
    Serial.print(":");
    Serial.print(ssp_link_key[i],HEX);
  }
  Serial.println("");
#endif

}

void loop() {

  Usb.Task();

#ifdef ENABLE_PAIR
  if ((Xb1s.linkkeyNotification() == true)&&(stored_ssp_status == false)) {
    // Get SSP Data.
    Xb1s.getConnectedAddressHex(ssp_bdaddr);
    Xb1s.getPairedLinkkeyHex(ssp_link_key);
    // Write SSP Data to EEPROM.
    for(uint8_t i = 0; i < 6; i++) {
      EEPROM.write(i, ssp_bdaddr[i]);
    }
    for(uint8_t i = 0; i < 16; i++) {
      EEPROM.write(i + 6, ssp_link_key[i]);
    }

    Serial.println("");
    Serial.print("connect Device Name ");
    Serial.println(Btdssp.remote_name);

    Serial.print("Stored Connected BD Address to EEPROM.");
    Serial.print(ssp_bdaddr[0],HEX);
    for(int i = 1 ; i < 6; i++) {
      Serial.print(":");
      Serial.print(ssp_bdaddr[i],HEX);
    }
    Serial.println("");
    Serial.print("Stored Generated SSP LinkKey to EEPROM.");
    Serial.print(ssp_link_key[0],HEX);
    for(int i = 1 ; i < 16; i++) {
      Serial.print(":");
      Serial.print(ssp_link_key[i],HEX);
    }
    Serial.println("");

    stored_ssp_status = true;
  }
#endif


  if (Xb1s.connected()) {
    if (Xb1s.stickData(STICK_LX) > (32767+7680) || Xb1s.stickData(STICK_LX) < (32767-7680) || Xb1s.stickData(STICK_LY) > (32767+7680) || Xb1s.stickData(STICK_LY) < (32767-7680)) {
      Serial.print(F("Stick_LX: "));
      Serial.print(Xb1s.stickData(STICK_LX));
      Serial.print(F("\tStick_LY: "));
      Serial.println(Xb1s.stickData(STICK_LY));
    }
    if (Xb1s.stickData(STICK_RX) > (32767+7680) || Xb1s.stickData(STICK_RX) < (32767-7680) || Xb1s.stickData(STICK_RY) > (32767+7680) || Xb1s.stickData(STICK_RY) < (32767-7680)) {
      Serial.print(F("Stick_RX: "));
      Serial.print(Xb1s.stickData(STICK_RX));
      Serial.print(F("\tStick_RY: "));
      Serial.println(Xb1s.stickData(STICK_RY));
    }

    if (Xb1s.triggerData(LT) > 0) {
      Serial.print(F("Trigger_LT: "));
      Serial.println(Xb1s.triggerData(LT));
    }
    if (Xb1s.triggerData(RT) > 0) {
      Serial.print(F("Trigger_RT: "));
      Serial.println(Xb1s.triggerData(RT));
    }

    int magLeft = map(Xb1s.triggerData(LT), 0, 1023, 0, 100);
    int magRight = map(Xb1s.triggerData(RT), 0, 1023, 0, 100);
    if( ! Xb1s.buttonData(LB)){  // If the rumble data transmit is not restrained,it will be crashed.
      Xb1s.rumble(magLeft, magRight, magLeft, magRight);
    }
/*
      Serial.print(F("magLeft: "));
      Serial.print(magLeft);
      Serial.print(F("\tmagRight: "));
      Serial.println(magRight);
*/

    if (Xb1s.dpadClick(DPAD_UP)) {
      Serial.println(F("DPAD_UP"));
      Xb1s.rumblePulseEnable(RUMBLE_TRIGGER_ONLY);
      Serial.println(F("Pulse_RUMBLE_TRIGGER_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_UP_RIGHT)) {
      Serial.println(F("DPAD_UP_RIGHT"));
      Xb1s.rumblePulseEnable(RUMBLE_RIGHT_TRIGGER);
      Serial.println(F("Pulse_RUMBLE_RIGHT_TRIGGER"));
    }
    if (Xb1s.dpadClick(DPAD_RIGHT)) {
      Serial.println(F("DPAD_RIGHT"));
      Xb1s.rumblePulseEnable(RUMBLE_RIGHT_ONLY);
      Serial.println(F("Pulse_RUMBLE_RIGHT_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_RIGHT_DOWN)) {
      Serial.println(F("DPAD_RIGHT_DOWN"));
      Xb1s.rumblePulseEnable(RUMBLE_RIGHT_WEAK);
      Serial.println(F("Pulse_RUMBLE_RIGHT_WEAK"));
    }
    if (Xb1s.dpadClick(DPAD_DOWN)) {
      Serial.println(F("DPAD_DOWN"));
      Xb1s.rumblePulseEnable(RUMBLE_MAIN_ONLY);
      Serial.println(F("Pulse_RUMBLE_MAIN_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_DOWN_LEFT)) {
      Serial.println(F("DPAD_DOWN_LEFT"));
      Xb1s.rumblePulseEnable(RUMBLE_LEFT_STRONG);
      Serial.println(F("Pulse_RUMBLE_LEFT_STRONG"));
    }
    if (Xb1s.dpadClick(DPAD_LEFT)) {
      Serial.println(F("DPAD_LEFT"));
      Xb1s.rumblePulseEnable(RUMBLE_LEFT_ONLY);
      Serial.println(F("Pulse_RUMBLE_LEFT_ONLY"));
    }
    if (Xb1s.dpadClick(DPAD_LEFT_UP)) {
      Serial.println(F("DPAD_LEFT_UP"));
      Xb1s.rumblePulseEnable(RUMBLE_LEFT_TRIGGER);
      Serial.println(F("Pulse_RUMBLE_LEFT_TRIGGER"));
    }     


    if (Xb1s.buttonClick(A)) {
      Serial.println(F("A"));
      Serial.println(F("SetRumblePulseDemoPattern A"));
      Xb1s.rumblePulseMagSet(50, 100, 50, 100);
      Xb1s.rumblePulseLoopSet(10, 5, 5);
    }
    if (Xb1s.buttonClick(B)) {
      Serial.println(F("B"));
      Serial.println(F("SetRumblePulseDemoPattern B"));
      Xb1s.rumblePulseMagSet(100, 100, 100, 100);
      Xb1s.rumblePulseLoopSet(50, 50, 2);
    }
    if (Xb1s.buttonClick(X)) {
      Serial.println(F("X"));
      Xb1s.rumblePulseEnable(RUMBLE_ALL);
      Serial.println(F("Pulse_RUMBLE_ALL"));
    }
    if (Xb1s.buttonClick(Y)) {
      Serial.println(F("Y"));
      Xb1s.rumbleStopAll();
      Serial.println(F("rumbleStopAll"));
      delay(100);
      Xb1s.rumbleInit();
      Serial.println(F("rumbleInit"));
    }

//    if (Xb1s.buttonClick(LB))
    if (Xb1s.buttonData(LB)){
      Serial.println(F("LB"));
    }
    int magAll = (Xb1s.buttonData(LB) * 100);
    if((magLeft == 0) && (magRight == 0)){ // If the rumble data transmit is not restrained,it will be crashed.
      Xb1s.rumble(magAll, magAll, magAll, magAll);
    }
    if (Xb1s.buttonClick(RB)) {
      Serial.println(F("RB"));
      Xb1s.rumblePulseStart();
      Serial.println(F("rumblePulseStart"));
    }
    if (Xb1s.buttonClick(RS))
      Serial.println(F("RS"));
    if (Xb1s.buttonClick(LS))
      Serial.println(F("LS"));


    if (Xb1s.buttonClick(VIEW)) {
      Serial.println(F("VIEW"));
      Serial.print(F("BatteryStatus:"));
      Serial.println(Xb1s.batteryStatus(), BIN);
      Serial.print(F("BatteryLevel:"));
      if(Xb1s.batteryLevel() == 3) Serial.println(F("Full"));
      else if(Xb1s.batteryLevel() == 2) Serial.println(F("High"));
      else if(Xb1s.batteryLevel() == 1) Serial.println(F("Normal"));
      else {
        Serial.print(F("Low:"));
        Serial.println(Xb1s.batteryLevel(), BIN);
      }
    }
    if (Xb1s.buttonClick(MENU)) {
      Serial.println(F("MENU"));
      Xb1s.rumbleTest(0b0110,50,50,50,50,100,200,2);
      Serial.println(F("rumbleTest"));
      Serial.println(F("0b0110,50,50,50,50,100,200,2"));
      Serial.println(F("RumbleMotorEnable:0b0110, Mag:50, VibrationTime:100x10ms, StartWaiting:200x10ms, LoopCount:2(Vibrate 3 times)"));
    }
    if (Xb1s.buttonClick(XBOX)) {
      Serial.println(F("XBOX"));
    }
  }
}
