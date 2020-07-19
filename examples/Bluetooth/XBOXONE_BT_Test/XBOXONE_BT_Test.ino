#include <BTXBOX.h>
#include <hiduniversal.h>
#include <usbhub.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include "xboxonebtparser.h"

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTDSSP Btdssp(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the class in two ways */
// This will start an inquiry and then pair with your device - you only have to do this once
BTXBOX btxbox(&Btdssp, PAIR);

// After that you can simply create the instance like so
//BTXBOX btxbox(&Btdssp);

HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser JoyPrs(&JoyEvents);

void setup() {
        Serial.begin(115200);
#if !defined(__MIPSEL__)
        while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
        Serial.println("Start");

        if (Usb.Init() == -1)
                Serial.println("OSC did not start.");

        delay(200);

        if (!Hid.SetReportParser(0, &JoyPrs))
                ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);

        btxbox.SetReportParser(GAMEPAD_PARSER_ID, &JoyPrs);

        // If "Boot Protocol Mode" does not work, then try "Report Protocol Mode"
        // If that does not work either, then uncomment PRINTREPORT in BTHID.cpp to see the raw report
        //bthid.setProtocolMode(USB_HID_BOOT_PROTOCOL); // Boot Protocol Mode
        btxbox.setProtocolMode(HID_RPT_PROTOCOL); // Report Protocol Mode

        Serial.println("XBOX Bluetooth Library Started");

}

void loop() {
        Usb.Task();
}
