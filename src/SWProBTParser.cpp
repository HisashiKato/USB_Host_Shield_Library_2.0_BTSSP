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

#include "SWProBTParser.h"

// To enable serial debugging see "settings.h"
//#define PRINTREPORT // Uncomment to print the report send by the SWPro Controller



uint16_t SWProBTParser::stickData(JoystickEnum a) {
        return swprobtReceivedData.joystick[(uint8_t)a];
}


uint8_t SWProBTParser::dpadData() {
        return swprobtReceivedData.dpad;
}

bool SWProBTParser::dpadClick(DpadEnum a){
        bool click = false;
        if(swprobtReceivedData.dpad == (uint8_t)a){
                if((pressedDpad != (uint8_t)a)||(pressedDpad == (uint8_t)DpadEnum::DPAD_NOT_PRESSED)){
                        click = true;
                        pressedDpad = (uint8_t)a;
                }
        }
        return click;
}


bool SWProBTParser::buttonData(ButtonEnum b) {
        return swprobtReceivedData.btn.value & pgm_read_word(&BUTTONMASK[(uint8_t)b]);
}

bool SWProBTParser::buttonClick(ButtonEnum b) {
        uint16_t mask = pgm_read_word(&BUTTONMASK[(uint8_t)b]);
        bool click = buttonClickState.value & mask;
        buttonClickState.value &= ~mask; // Clear "click" event
        return click;
}


void SWProBTParser::Parse(uint8_t len, uint8_t *buf) {
        if (len > 1 && buf)  {
#ifdef PRINTREPORT
                Notify(PSTR("\r\n"), 0x80);
                for (uint8_t i = 0; i < len; i++) {
                        D_PrintHex<uint8_t > (buf[i], 0x80);
                        Notify(PSTR(" "), 0x80);
                }
#endif
                if (buf[0] == 0x3F) { // Check report ID
                        memcpy(&swprobtReceivedData, buf + 1, min((uint8_t)(len - 1), MFK_CASTUINT8T sizeof(swprobtReceivedData)));

                        if (swprobtReceivedData.btn.value != oldButtonState.value) { // Check if anything button has changed
                                buttonClickState.value = swprobtReceivedData.btn.value & ~oldButtonState.value; // Update click state variable
                                oldButtonState.value = swprobtReceivedData.btn.value;
                        }
                        //dpadcheck
                        if (swprobtReceivedData.dpad == (uint8_t)DpadEnum::DPAD_NOT_PRESSED) {
                                pressedDpad = (uint8_t)DpadEnum::DPAD_NOT_PRESSED;
                        }

                } else if (buf[0] == 0x21) { // BT_HIDP_SW_SUBCMD_ACK

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





void SWProBTParser::Reset() {
        for (uint8_t i = 0; i < 4; i++)
                swprobtReceivedData.joystick[i] = 32767; // Center value
        swprobtReceivedData.dpad = (uint8_t)DpadEnum::DPAD_NOT_PRESSED;
        pressedDpad = (uint8_t)DpadEnum::DPAD_NOT_PRESSED;
        swprobtReceivedData.btn.value = 0;
};

