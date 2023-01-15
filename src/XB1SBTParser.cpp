/* Copyright (C) 2014 Kristian Lauszus, TKJ Electronics. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com
 
 Modified 26 Feb 2021 by HisashiKato
 Web      :  http://kato-h.cocolog-nifty.com/khweblog/
*/

#include "XB1SBTParser.h"

// To enable serial debugging see "settings.h"
//#define PRINTREPORT // Uncomment to print the report send by the XBOX ONE S Controller


uint16_t XB1SBTParser::stickData(XB1S_JoystickEnum a) {
        return xb1sbtReceivedData.joystick[(uint8_t)a];
}


uint16_t XB1SBTParser::triggerData(XB1S_TriggerEnum a) {
        return xb1sbtReceivedData.trigger[(uint8_t)a];
}


uint8_t XB1SBTParser::dpadData() {
        return xb1sbtReceivedData.dpad;
}

bool XB1SBTParser::dpadClick(XB1S_DpadEnum a) {
        bool click = false;
        if(xb1sbtReceivedData.dpad == (uint8_t)a) {
                if((pressedDpad != (uint8_t)a)||(pressedDpad == (uint8_t)XB1S_DpadEnum::DPAD_NOT_PRESSED)) {
                        click = true;
                        pressedDpad = (uint8_t)a;
                }
        }
        return click;
}


bool XB1SBTParser::buttonData(XB1S_ButtonEnum b) {
        if (controllerFW == FW_31) 
                buttonMask = pgm_read_dword(&XB1SBTBUTTONMASK31[(uint8_t)b]);
        else 
                buttonMask = pgm_read_dword(&XB1SBTBUTTONMASK48[(uint8_t)b]);

        return (xb1sbtReceivedData.btn.value & buttonMask);
}

bool XB1SBTParser::buttonClick(XB1S_ButtonEnum b) {
        if (controllerFW == FW_31) 
                buttonMask = pgm_read_dword(&XB1SBTBUTTONMASK31[(uint8_t)b]);
        else 
                buttonMask = pgm_read_dword(&XB1SBTBUTTONMASK48[(uint8_t)b]);

        bool click = buttonClickState.value & buttonMask;
        buttonClickState.value &= ~buttonMask; // Clear "click" event
        return click;
}


uint8_t XB1SBTParser::batteryLevel() {
        return xb1sbtBatteryStatus.batterylevel;
}

uint8_t XB1SBTParser::batteryStatus() {
        return xb1sbtBatteryStatus.data;
}


void XB1SBTParser::Parse(uint8_t len, uint8_t *buf) {
        if (len > 1 && buf)  {
#ifdef PRINTREPORT
                Notify(PSTR("\r\n"), 0x80);
                for (uint8_t i = 0; i < len; i++) {
                        D_PrintHex<uint8_t > (buf[i], 0x80);
                        Notify(PSTR(" "), 0x80);
                }
#endif
                if (buf[0] == 0x01) { // Check report ID
                        memcpy(&xb1sbtReceivedData, buf + 1, min((uint8_t)(len - 1), MFK_CASTUINT8T sizeof(xb1sbtReceivedData)));

                        if (len == 16) {
                                controllerFW = FW_31;
                        } else {
                                controllerFW = FW_48;
                        }

                        if (xb1sbtReceivedData.btn.value != oldButtonState.value) { // Check if anything button has changed
                                buttonClickState.value = xb1sbtReceivedData.btn.value & ~oldButtonState.value; // Update click state variable
                                oldButtonState.value = xb1sbtReceivedData.btn.value;
                        }

                        //dpadcheck
                        if (xb1sbtReceivedData.dpad == (uint8_t)XB1S_DpadEnum::DPAD_NOT_PRESSED) {
                                pressedDpad = (uint8_t)XB1S_DpadEnum::DPAD_NOT_PRESSED;
                        }

                } else if (buf[0] == 0x02) { // This report contains the Xbox button
                        if (buf[1] == 0x01) {
                                xb1sbtReceivedData.btn.value |= pgm_read_dword(&XB1SBTBUTTONMASK31[XB1S_ButtonEnum::XBOX]);
                        } else {
                                xb1sbtReceivedData.btn.value &= ~pgm_read_dword(&XB1SBTBUTTONMASK31[XB1S_ButtonEnum::XBOX]);
                        }
                                buttonClickState.value = xb1sbtReceivedData.btn.value;
                                oldButtonState.value = xb1sbtReceivedData.btn.value;
                        return;

                } else if (buf[0] == 0x04) { // Battery status
                        xb1sbtBatteryStatus.data = buf[1];
                        return;

                } else {
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nUnsupported report id: "), 0x80);
                        D_PrintHex<uint8_t > (buf[0], 0x80);
#endif
                        return;
                }
        }
}


void XB1SBTParser::Reset() {
        for (uint8_t i = 0; i < 4; i++)
                xb1sbtReceivedData.joystick[i] = 32767; // Center value
        for (uint8_t i = 0; i < 2; i++)
                xb1sbtReceivedData.trigger[i] = 0;
        xb1sbtReceivedData.dpad = (uint8_t)XB1S_DpadEnum::DPAD_NOT_PRESSED;
        pressedDpad = (uint8_t)XB1S_DpadEnum::DPAD_NOT_PRESSED;
        xb1sbtReceivedData.btn.value = 0;
        oldButtonState.value = 0;
        xb1sbtBatteryStatus.data = 0;
};

