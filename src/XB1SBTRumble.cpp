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

#include "XB1SBTRumble.h"

// To enable serial debugging see "settings.h"
//#define PRINTREPORT // Uncomment to print the report send by the XBOX ONE S Controller


#define XB1S_RUMBLE_REPORT_ID 0x03  //buf[1] = 0x03; // Report ID
#define XB1S_RUMBLE_REPORT_SIZE 8


void XB1SBTRumble::rumbleInit() {
        Reset();
        sendRumbleData(&xb1sbtRumbleData);
}


void XB1SBTRumble::rumbleEnable(RumbleMotorsEnableEnum a) {
        if(oldRumbleData.motorEnable != (uint8_t)a){
                xb1sbtRumbleData.motorEnable = (uint8_t)a;
                
                sendRumbleData(&xb1sbtRumbleData);
                
                oldRumbleData.motorEnable = xb1sbtRumbleData.motorEnable;
        }
}


void XB1SBTRumble::rumble(uint8_t maglt, uint8_t magrt, uint8_t magls, uint8_t magrw) {

        xb1sbtRumbleData.mag.leftTrigger = maglt;
        xb1sbtRumbleData.mag.rightTrigger = magrt;
        xb1sbtRumbleData.mag.leftStrong = magls;
        xb1sbtRumbleData.mag.rightWeak = magrw;

        if(oldRumbleData.mag.value != xb1sbtRumbleData.mag.value){
                
                sendRumbleData(&xb1sbtRumbleData);
                
                oldRumbleData.mag.value = xb1sbtRumbleData.mag.value;
        }
}


void XB1SBTRumble::rumblePulseEnable(RumbleMotorsEnableEnum a) {

        xb1sbtRumblePluse.motorEnable = (uint8_t)a;
}


void XB1SBTRumble::rumblePulseMagSet(uint8_t maglt, uint8_t magrt, uint8_t magls, uint8_t magrw) {

        xb1sbtRumblePluse.mag.leftTrigger = maglt;
        xb1sbtRumblePluse.mag.rightTrigger = magrt;
        xb1sbtRumblePluse.mag.leftStrong = magls;
        xb1sbtRumblePluse.mag.rightWeak = magrw;

}

void XB1SBTRumble::rumblePulseLoopSet(uint8_t plsduration, uint8_t plsdelay, uint8_t loopcnt) {

        xb1sbtRumblePluse.pulseDuration = plsduration;
        xb1sbtRumblePluse.pulseDelay = plsdelay;
        xb1sbtRumblePluse.loopCount = loopcnt;

}


void XB1SBTRumble::rumblePulseStart() {

        sendRumbleData(&xb1sbtRumblePluse);

}


void XB1SBTRumble::rumbleStopAll() {

        xb1sbtRumbleData.motorEnable = RUMBLE_ALL;
        xb1sbtRumbleData.mag.value = 0;
        xb1sbtRumbleData.pulseDuration = 0;
        xb1sbtRumbleData.pulseDelay = 0;
        xb1sbtRumbleData.loopCount = 0;

        sendRumbleData(&xb1sbtRumbleData);

        Reset();
}

void XB1SBTRumble::rumbleTest(uint8_t m,
                              uint8_t maglt, uint8_t magrt, uint8_t magls, uint8_t magrw,
                              uint8_t plsduration, uint8_t plsdelay, uint8_t loopcnt) {

        xb1sbtRumbleData.motorEnable = m;
        xb1sbtRumbleData.mag.leftTrigger = maglt;
        xb1sbtRumbleData.mag.rightTrigger = magrt;
        xb1sbtRumbleData.mag.leftStrong = magls;
        xb1sbtRumbleData.mag.rightWeak = magrw;
        xb1sbtRumbleData.pulseDuration = plsduration;
        xb1sbtRumbleData.pulseDelay = plsdelay;
        xb1sbtRumbleData.loopCount = loopcnt;

        sendRumbleData(&xb1sbtRumbleData);
}



void XB1SBTRumble::sendRumbleData(XB1SBTRumbleData *sendData) {
        uint8_t len = XB1S_RUMBLE_REPORT_SIZE + 2;
        uint8_t buf[len];
        buf[0] = 0xA2;                  // HID BT DATA (0xA0) | Report Type (Output 0x02)
        buf[1] = XB1S_RUMBLE_REPORT_ID; // ID 0x03
        memcpy( &buf[2], (void *)sendData, (size_t)XB1S_RUMBLE_REPORT_SIZE);
#ifdef PRINTREPORT
        Notify(PSTR("\r\nsendRumbleData: "), 0x80);
        for(uint8_t i = 0; i < len; i++) {
                D_PrintHex<uint8_t > (buf[i], 0x80);
                Notify(PSTR(" "), 0x80);
        }
#endif
        sendReport(buf, len);
}




void XB1SBTRumble::Reset() {
        xb1sbtRumbleData.motorEnable = RUMBLE_ALL;
        xb1sbtRumbleData.mag.value = 0;
        xb1sbtRumbleData.pulseDuration = 0xFF;
        xb1sbtRumbleData.pulseDelay = 0;
        xb1sbtRumbleData.loopCount = 0xEB;

        oldRumbleData = xb1sbtRumbleData;
}
