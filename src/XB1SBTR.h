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
 
*/

#ifndef _xb1sbtr_h_
#define _xb1sbtr_h_

#include "BTHIDs.h"
#include "XB1SBTParser.h"
#include "XB1SBTRumble.h"


#define STICK_LX  XB1S_STICK_LX //X_Axis     // left_stick_x
#define STICK_LY  XB1S_STICK_LY //Y_Axis     // left_stick_y
#define STICK_RX  XB1S_STICK_RX //X_Rotation // right_stick_x
#define STICK_RY  XB1S_STICK_RY //Y_Rotation // right_stick_y

#define LT        XB1S_LT  //Z_Axis     //trigger_left
#define RT        XB1S_RT  //Z_Rotation //trigger_right


#define DPAD_NOT_PRESSED  XB1S_DPAD_NOT_PRESSED
#define DPAD_UP           XB1S_DPAD_UP
#define DPAD_UP_RIGHT     XB1S_DPAD_UP_RIGHT
#define DPAD_RIGHT        XB1S_DPAD_RIGHT
#define DPAD_RIGHT_DOWN   XB1S_DPAD_RIGHT_DOWN
#define DPAD_DOWN         XB1S_DPAD_DOWN
#define DPAD_DOWN_LEFT    XB1S_DPAD_DOWN_LEFT
#define DPAD_LEFT         XB1S_DPAD_LEFT
#define DPAD_LEFT_UP      XB1S_DPAD_LEFT_UP


#define A     XB1S_A
#define B     XB1S_B
#define X     XB1S_X
#define Y     XB1S_Y
#define LB    XB1S_LB
#define RB    XB1S_RB
#define VIEW  XB1S_VIEW
#define MENU  XB1S_MENU
#define LS    XB1S_LS
#define RS    XB1S_RS
#define XBOX  XB1S_XBOX


#define RUMBLE_NONE           XB1S_RUMBLE_NONE          // 0x00
#define RUMBLE_RIGHT_WEAK     XB1S_RUMBLE_RIGHT_WEAK    // 0x01
#define RUMBLE_LEFT_STRONG    XB1S_RUMBLE_LEFT_STRONG   // 0x02
#define RUMBLE_MAIN_ONLY      XB1S_RUMBLE_MAIN_ONLY     // LEFT_STRONG | RIGHT_WEAK

#define RUMBLE_RIGHT_TRIGGER  XB1S_RUMBLE_RIGHT_TRIGGER // 0x04
#define RUMBLE_LEFT_TRIGGER   XB1S_RUMBLE_LEFT_TRIGGER  // 0x08
#define RUMBLE_TRIGGER_ONLY   XB1S_RUMBLE_TRIGGER_ONLY  // LEFT_TRIGGER | RIGHT_TRIGGER

#define RUMBLE_RIGHT_ONLY     XB1S_RUMBLE_RIGHT_ONLY    // RIGHT_TRIGGER | RIGHT_WEAK
#define RUMBLE_LEFT_ONLY      XB1S_RUMBLE_LEFT_ONLY     // LEFT_TRIGGER | LEFT_STRONG

#define RUMBLE_ALL            XB1S_RUMBLE_ALL           // 0x0F



/**
 * This class implements support for the XB1S controller via Bluetooth.
 * It uses the BTHIDs class for all the Bluetooth communication.
 */
class XB1SBTR : public BTHIDs, public XB1SBTParser, public XB1SBTRumble {
public:
        /**
         * Constructor for the XB1SBT class.
         * @param  p     Pointer to the BTDSSP class instance.
         * @param  pair  Set this to true in order to pair with the device. If the argument is omitted then it will not pair with it. One can use ::PAIR to set it to true.
         */
        XB1SBTR(BTDSSP *p, bool pair = false) :
        BTHIDs(p, pair) {
                XB1SBTParser::Reset();
                XB1SBTRumble::Reset();
        };

        /**
         * Used to check if a XB1S controller is connected.
         * @return Returns true if it is connected.
         */
        bool connected() {
                return BTHIDs::connected;
        };

protected:
        /** @name BTHIDs implementation */
        /**
         * Used to parse Bluetooth HID data.
         * @param len The length of the incoming data.
         * @param buf Pointer to the data buffer.
         */
        virtual void ParseBTHIDsData(uint8_t len, uint8_t *buf) {
                XB1SBTParser::Parse(len, buf);
        };

        /**
         * Called when a device is successfully initialized.
         * Use attachOnInit(void (*funcOnInit)(void)) to call your own function.
         * This is useful for instance if you want to set the LEDs in a specific way.
         */
        virtual void OnInitBTHIDs() {
                XB1SBTParser::Reset();
                XB1SBTRumble::rumbleInit();
        };

        /** Used to reset the different buffers to there default values */
        virtual void ResetBTHIDs() {
                XB1SBTParser::Reset();
                XB1SBTRumble::Reset();
        };
        /**@}*/

        /** @name XB1SBTParser implementation */

        virtual void sendReport(uint8_t *data, uint8_t datasize) {
                BTHIDs::sendReport(data, datasize);
        };
        /**@}*/

private:

};
#endif
