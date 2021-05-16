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
 
 Modified 4 Apr 2021 by HisashiKato
 Web      :  http://kato-h.cocolog-nifty.com/khweblog/
*/

#include "SWProBTParser.h"

// To enable serial debugging see "settings.h"
//#define PRINTREPORT // Uncomment to print the report send by the SWPro Controller

#define SWPro_SEND_CONFIG_ID 0x01 //send Rumble and subcommand.
#define SWPro_SEND_CONFIG_SIZE 48

#define SWProBT_SUBCMD_ACK             0x21

#define SWProBT_SUBCMD_SET_REPORT_MODE 0x03
#define SWProBT_SUBCMD_SET_LED         0x30
#define SWProBT_SUBCMD_EABLE_RUMBLE    0x48

#define SWProBT_STANDARD_FULL 0x30 // x30 Standard full mode
#define SWProBT_NFC_IR        0x31 // x31 NFC/IR mode
#define SWProBT_SIMPLE_HID    0x3F // x3F Simple HID mode


uint16_t SWProBTParser::stickData(SWPro_JoystickEnum a) {
        return swprobtReceivedData.joystick[(uint8_t)a];
}


uint8_t SWProBTParser::dpadData() {
        return swprobtReceivedData.dpad;
}

bool SWProBTParser::dpadClick(SWPro_DpadEnum a){
        bool click = false;
        if(swprobtReceivedData.dpad == (uint8_t)a){
                if((pressedDpad != (uint8_t)a)||(pressedDpad == (uint8_t)SWPro_DpadEnum::SWPro_DPAD_NOT_PRESSED)){
                        click = true;
                        pressedDpad = (uint8_t)a;
                }
        }
        return click;
}


bool SWProBTParser::buttonData(SWPro_ButtonEnum b) {
        return swprobtReceivedData.btn.value & pgm_read_word(&SWProBUTTONMASK[(uint8_t)b]);
}

bool SWProBTParser::buttonClick(SWPro_ButtonEnum b) {
        uint16_t mask = pgm_read_word(&SWProBUTTONMASK[(uint8_t)b]);
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
                // Check report ID
                if (buf[0] == SWProBT_SIMPLE_HID) { // ID 0x3F 
                        memcpy(&swprobtReceivedData, buf + 1, min((uint8_t)(len - 1), MFK_CASTUINT8T sizeof(swprobtReceivedData)));

                        if (swprobtReceivedData.btn.value != oldButtonState.value) { // Check if anything button has changed
                                buttonClickState.value = swprobtReceivedData.btn.value & ~oldButtonState.value; // Update click state variable
                                oldButtonState.value = swprobtReceivedData.btn.value;
                        }
                        //dpadcheck
                        if (swprobtReceivedData.dpad == (uint8_t)SWPro_DpadEnum::SWPro_DPAD_NOT_PRESSED) {
                                pressedDpad = (uint8_t)SWPro_DpadEnum::SWPro_DPAD_NOT_PRESSED;
                        }

                } else if (buf[0] == SWProBT_SUBCMD_ACK) { // ID 0x21 
                        memcpy(&swprobtReceivedSubCommandAck, buf + 1, min((uint8_t)(len - 1), MFK_CASTUINT8T sizeof(swprobtReceivedSubCommandAck)));

                        switch(swprobtReceivedSubCommandAck.replySubCommand) {
/*
                                case SWProBT_SUBCMD_SET_LED:
                                        swprobtSendConfigData.gpnum = sw_gpnum;
                                        swprobtSendConfigData.subCommand = SWProBT_SUBCMD_EABLE_RUMBLE;
                                        swprobtSendConfigData.subCommandData[0] = 0x00; // 0x00:Disable 0x01:Enable

                                        sendConfigData(&swprobtSendConfigData);
                                        break;
*/
                                case SWProBT_SUBCMD_EABLE_RUMBLE:
                                        swprobtSendConfigData.gpnum = sw_gpnum;
                                        swprobtSendConfigData.subCommand = SWProBT_SUBCMD_SET_REPORT_MODE;
                                        swprobtSendConfigData.subCommandData[0] = SWProBT_SIMPLE_HID;

                                        sendConfigData(&swprobtSendConfigData);
                                        break;

                                case SWProBT_SUBCMD_SET_REPORT_MODE:
                                        break;

                        }

                } else {
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nUnsupported report id: "), 0x80);
                        D_PrintHex<uint8_t > (buf[0], 0x80);
#endif
                        return;
                }
        }
}


void SWProBTParser::sendConfigData(SWProBTSendConfigData *sendData) { //send Rumble & SubCommand
        uint8_t len = SWPro_SEND_CONFIG_SIZE + 2;
        uint8_t buf[len];
        buf[0] = 0xA2;                 // HID BT DATA (0xA0) | Report Type (Output 0x02)
        buf[1] = SWPro_SEND_CONFIG_ID; // ID 0x01
        memcpy( &buf[2], (void *)sendData, (size_t)SWPro_SEND_CONFIG_SIZE);
        sw_gpnum++;
        if (sw_gpnum == 0x10) {
                sw_gpnum = 0;
        }
#ifdef PRINTREPORT
        Notify(PSTR("\r\nsendConfigData: "), 0x80);
        for(uint8_t i = 0; i < len; i++) {
                D_PrintHex<uint8_t > (buf[i], 0x80);
                Notify(PSTR(" "), 0x80);
        }
#endif
        sendReport(buf, len);
}


void SWProBTParser::sendReport(uint8_t *data, uint8_t datasize) {

}


void SWProBTParser::setPlayerLED(uint8_t led) {
        swprobtSendConfigData.gpnum = sw_gpnum;
        swprobtSendConfigData.subCommand = SWProBT_SUBCMD_SET_LED;
        swprobtSendConfigData.subCommandData[0] = led;

        sendConfigData(&swprobtSendConfigData);
}

void SWProBTParser::simpleRumbleL(bool l) {
        rumbleL = l;
        if (rumbleL == 1){
                memcpy((void *)swprobtSendConfigData.rumbleDataL, (void *)sw_rumble_on, sizeof(sw_rumble_on));
        } else {
                memcpy((void *)swprobtSendConfigData.rumbleDataL, (void *)sw_rumble_off, sizeof(sw_rumble_off));
        }
        setSimpleRumble();
}

void SWProBTParser::simpleRumbleR(bool r) {
        rumbleR = r;
        if (rumbleR == 1){
                memcpy((void *)swprobtSendConfigData.rumbleDataR, (void *)sw_rumble_on, sizeof(sw_rumble_on));
        } else {
                memcpy((void *)swprobtSendConfigData.rumbleDataR, (void *)sw_rumble_off, sizeof(sw_rumble_off));
        }
        setSimpleRumble();
}

void SWProBTParser::setSimpleRumble() {

        if ((oldRumbleL != rumbleL)||(oldRumbleR != rumbleR)) {
                sendConfigData(&swprobtSendConfigData);
                oldRumbleL = rumbleL;
                oldRumbleR = rumbleR;
        }
}




void SWProBTParser::Init() {
        setPlayerLED(0b00001111); // 0b00001111

        swprobtSendConfigData.gpnum = sw_gpnum;
        swprobtSendConfigData.subCommand = SWProBT_SUBCMD_EABLE_RUMBLE;
        swprobtSendConfigData.subCommandData[0] = 0x01; // 0x00:Disable 0x01:Enable

        sendConfigData(&swprobtSendConfigData);
}

void SWProBTParser::Reset() {
        for (uint8_t i = 0; i < 4; i++)
                swprobtReceivedData.joystick[i] = 32767; // Center value
        swprobtReceivedData.dpad = (uint8_t)SWPro_DpadEnum::SWPro_DPAD_NOT_PRESSED;
        pressedDpad = (uint8_t)SWPro_DpadEnum::SWPro_DPAD_NOT_PRESSED;
        swprobtReceivedData.btn.value = 0;
}

