/*
 Example sketch for the HID Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
   
 Modified 18 July 2020 by HisashiKato
 Web      :  http://kato-h.cocolog-nifty.com/khweblog/
*/

#include <BTHIDa.h>
#include <usbhub.h>
#include <EEPROM.h>
#include "KeyboardParser.h"
#include "MouseParser.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

//#define ENABLE_PAIR

// Remove "//" from the "//#define ENABLE_PAIR" line, enable "#define ENABLE_PAIR",
// and write this sketch to the Arduino to enter the pairing state.
//
// Put your device in pairing mode.
//
// After pairing is complete, add "//" to the "#define ENABLE_PAIR" line to
// make it "//#define ENABLE_PAIR" and write this sketch to the "Arduino" again.
// After that, simply turn on power each other and they will be connected.
//
// (This used a translation site to translate it into English.)


USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so
BTHIDa bthida(&Btdssp);

KbdRptParser keyboardPrs;
MouseRptParser mousePrs;

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

  bthida.SetReportParser(KEYBOARD_PARSER_ID, &keyboardPrs);
  bthida.SetReportParser(MOUSE_PARSER_ID, &mousePrs);

  // If "Boot Protocol Mode" does not work, then try "Report Protocol Mode"
  // If that does not work either, then uncomment PRINTREPORT in BTHIDa.cpp to see the raw report
  bthida.setProtocolMode(USB_HID_BOOT_PROTOCOL); // Boot Protocol Mode
  //bthida.setProtocolMode(HID_RPT_PROTOCOL); // Report Protocol Mode

  Serial.println(F("HID Bluetooth Library Started"));

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
  bthida.setConnectAddress(ssp_bdaddr);
  bthida.setPairedLinkkey(ssp_link_key);

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
  if ((bthida.linkkeyNotification() == true)&&(stored_ssp_status == false)) {
    // Get SSP Data.
    bthida.getConnectedAddressHex(ssp_bdaddr);
    bthida.getPairedLinkkeyHex(ssp_link_key);
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
}
