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
// Press and hold the Sync button on the top of the Switch Pro Controller for a few seconds
// until the indicator lights start flashing that it is in pairing mode.
//
// After pairing is complete, add "//" to the "#define ENABLE_PAIR" line to
// make it "//#define ENABLE_PAIR" and write this sketch to the "Arduino" again.
// After that, simply turn on power each other and they will be connected.
//
// (This used a translation site to translate it into English.)

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so
SWProBT SWPro(&Btdssp);

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
  Serial.println(F("SWPro Bluetooth Library Started"));

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
  SWPro.setConnectAddress(ssp_bdaddr);
  SWPro.setPairedLinkkey(ssp_link_key);

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
  if ((SWPro.linkkeyNotification() == true)&&(stored_ssp_status == false)) {
    // Get SSP Data.
    SWPro.getConnectedAddressHex(ssp_bdaddr);
    SWPro.getPairedLinkkeyHex(ssp_link_key);
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


  if (SWPro.connected()) {
    if (SWPro.stickData(STICK_LX) > (32767+7680) || SWPro.stickData(STICK_LX) < (32767-7680) || SWPro.stickData(STICK_LY) > (32767+7680) || SWPro.stickData(STICK_LY) < (32767-7680)) {
      Serial.print(F("Stick_LX: "));
      Serial.print(SWPro.stickData(STICK_LX));
      Serial.print(F("\tStick_LY: "));
      Serial.println(SWPro.stickData(STICK_LY));
    }
    if (SWPro.stickData(STICK_RX) > (32767+7680) || SWPro.stickData(STICK_RX) < (32767-7680) || SWPro.stickData(STICK_RY) > (32767+7680) || SWPro.stickData(STICK_RY) < (32767-7680)) {
      Serial.print(F("Stick_RX: "));
      Serial.print(SWPro.stickData(STICK_RX));
      Serial.print(F("\tStick_RY: "));
      Serial.println(SWPro.stickData(STICK_RY));
    }


    if (SWPro.dpadClick(DPAD_UP))
      Serial.println(F("DPAD_UP"));
    if (SWPro.dpadClick(DPAD_UP_RIGHT))
      Serial.println(F("DPAD_UP_RIGHT"));
    if (SWPro.dpadClick(DPAD_RIGHT))
      Serial.println(F("DPAD_RIGHT"));
    if (SWPro.dpadClick(DPAD_RIGHT_DOWN))
      Serial.println(F("DPAD_RIGHT_DOWN"));
    if (SWPro.dpadClick(DPAD_DOWN))
      Serial.println(F("DPAD_DOWN"));
    if (SWPro.dpadClick(DPAD_DOWN_LEFT))
      Serial.println(F("DPAD_DOWN_LEFT"));
    if (SWPro.dpadClick(DPAD_LEFT))
      Serial.println(F("DPAD_LEFT"));
    if (SWPro.dpadClick(DPAD_LEFT_UP))
      Serial.println(F("DPAD_LEFT_UP"));


    if (SWPro.buttonClick(A)) {
      Serial.println(F("A"));
    }
    if (SWPro.buttonClick(B)) {
      Serial.println(F("B"));
    }
    if (SWPro.buttonClick(X)) {
      Serial.println(F("X"));
    }
    if (SWPro.buttonClick(Y)) {
      Serial.println(F("Y"));
    }

    if (SWPro.buttonClick(LB))
      Serial.println(F("LB"));
    if (SWPro.buttonClick(RB))
      Serial.println(F("RB"));

    if (SWPro.buttonClick(ZL))
      Serial.println(F("ZL"));
    if (SWPro.buttonData(ZL)) {
      SWPro.simpleRumbleL(1);
    }else{
      SWPro.simpleRumbleL(0);
    } 

    if (SWPro.buttonClick(ZR))
      Serial.println(F("ZR"));
    if (SWPro.buttonData(ZR)) {
      SWPro.simpleRumbleR(1);
    }else{
      SWPro.simpleRumbleR(0);
    } 


    if (SWPro.buttonClick(MINUS))
      Serial.println(F("MINUS"));
    if (SWPro.buttonClick(PLUS))
      Serial.println(F("PLUS"));

    if (SWPro.buttonClick(LS))
      Serial.println(F("LS"));
    if (SWPro.buttonClick(RS))
      Serial.println(F("RS"));


    if (SWPro.buttonClick(HOME)) {
      Serial.println(F("HOME"));
    }
    if (SWPro.buttonClick(CAPTURE)) {
      Serial.println(F("CAPTURE"));
    }

  }
}
