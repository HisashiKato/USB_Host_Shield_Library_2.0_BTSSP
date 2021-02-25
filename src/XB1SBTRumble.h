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

#ifndef _xb1sbtrumble_h_
#define _xb1sbtrumble_h_

#include "Usb.h"


// rumble motors enable bits
enum RumbleMotorsEnableEnum {
        RUMBLE_NONE = 0b0000,          // 0x00
        RUMBLE_RIGHT_WEAK = 0b0001,    // 0x01
        RUMBLE_LEFT_STRONG = 0b0010,   // 0x02
        RUMBLE_MAIN_ONLY = 0b0011,     // LEFT_STRONG | RIGHT_WEAK

        RUMBLE_RIGHT_TRIGGER = 0b0100, // 0x04
        RUMBLE_LEFT_TRIGGER = 0b1000,  // 0x08
        RUMBLE_TRIGGER_ONLY = 0b1100,  // LEFT_TRIGGER | RIGHT_TRIGGER

        RUMBLE_RIGHT_ONLY = 0b0101,    // RIGHT_TRIGGER | RIGHT_WEAK
        RUMBLE_LEFT_ONLY = 0b1010,     // LEFT_TRIGGER | LEFT_STRONG

        RUMBLE_ALL = 0b1111            // 0x0F
};



union RumbleMagnitude {
        struct {
                uint8_t leftTrigger;
                uint8_t rightTrigger;
                uint8_t leftStrong;
                uint8_t rightWeak;
        } __attribute__((packed));

        uint32_t value;

} __attribute__((packed));


struct XB1SBTRumbleData {
        uint8_t motorEnable;
        RumbleMagnitude mag;
        uint8_t pulseDuration; //sustain
        uint8_t pulseDelay;    //release
        uint8_t loopCount;
} __attribute__((packed));



class XB1SBTRumble {
public:

        void rumbleInit();

        void rumbleEnable(RumbleMotorsEnableEnum a);

        void rumble(uint8_t maglt, uint8_t magrt, uint8_t magls, uint8_t magrw);

        void rumblePulseEnable(RumbleMotorsEnableEnum a);

        void rumblePulseMagSet(uint8_t maglt, uint8_t magrt, uint8_t magls, uint8_t magrw);

        void rumblePulseLoopSet(uint8_t plsduration, uint8_t plsdelay, uint8_t loopcnt);

        void rumblePulseStart();

        void rumbleStopAll();

        void rumbleTest(uint8_t m,
                        uint8_t maglt, uint8_t magrt, uint8_t magls, uint8_t magrw,
                        uint8_t plsduration, uint8_t plsdelay, uint8_t loopcnt);

        void sendRumbleData(XB1SBTRumbleData *sendData);


protected:
        /** Used to reset the different buffers to their default values */
        void Reset();

        /**
         * Send the output to the Xbox One S controller.
         * @param data Pointer to data buffer.
         * @param datasize Bytes to send.
         */
        virtual void sendReport(uint8_t *data, uint8_t datasize);


private:
        XB1SBTRumbleData xb1sbtRumbleData,xb1sbtRumblePluse,oldRumbleData;
};
#endif
