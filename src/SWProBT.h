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

#ifndef _swprobt_h_
#define _swprobt_h_

#include "BTHIDs.h"
#include "SWProBTParser.h"


#define STICK_LX  SWPro_STICK_LX  //X_Axis     // left_stick_x
#define STICK_LY  SWPro_STICK_LY  //Y_Axis     // left_stick_y
#define STICK_RX  SWPro_STICK_RX  //X_Rotation // right_stick_x
#define STICK_RY  SWPro_STICK_RY  //Y_Rotation // right_stick_y

#define DPAD_UP           SWPro_DPAD_UP
#define DPAD_UP_RIGHT     SWPro_DPAD_UP_RIGHT
#define DPAD_RIGHT        SWPro_DPAD_RIGHT
#define DPAD_RIGHT_DOWN   SWPro_DPAD_RIGHT_DOWN
#define DPAD_DOWN         SWPro_DPAD_DOWN
#define DPAD_DOWN_LEFT    SWPro_DPAD_DOWN_LEFT
#define DPAD_LEFT         SWPro_DPAD_LEFT
#define DPAD_LEFT_UP      SWPro_DPAD_LEFT_UP
#define DPAD_NOT_PRESSED  SWPro_DPAD_NOT_PRESSED

#define B        SWPro_B
#define A        SWPro_A
#define Y        SWPro_Y
#define X        SWPro_X
#define LB       SWPro_LB
#define RB       SWPro_RB
#define ZL       SWPro_ZL
#define ZR       SWPro_ZR
#define MINUS    SWPro_MINUS
#define PLUS     SWPro_PLUS
#define LS       SWPro_LS
#define RS       SWPro_RS
#define HOME     SWPro_HOME
#define CAPTURE  SWPro_CAPTURE


/**
 * This class implements support for the SWPro controller via Bluetooth.
 * It uses the BTHIDs class for all the Bluetooth communication.
 */
class SWProBT : public BTHIDs, public SWProBTParser {
public:
        /**
         * Constructor for the SWProBT class.
         * @param  p     Pointer to the BTDSSP class instance.
         * @param  pair  Set this to true in order to pair with the device. If the argument is omitted then it will not pair with it. One can use ::PAIR to set it to true.
         */
        SWProBT(BTDSSP *p, bool pair = false) :
        BTHIDs(p, pair) {
                SWProBTParser::Reset();
        };

        /**
         * Used to check if a SWPro controller is connected.
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
                SWProBTParser::Parse(len, buf);
        };

        /**
         * Called when a device is successfully initialized.
         * Use attachOnInit(void (*funcOnInit)(void)) to call your own function.
         * This is useful for instance if you want to set the LEDs in a specific way.
         */
        virtual void OnInitBTHIDs() {
                SWProBTParser::Reset();
                SWProBTParser::Init();
        };

        /** Used to reset the different buffers to there default values */
        virtual void ResetBTHIDs() {
                SWProBTParser::Reset();
        };

        virtual void sendReport(uint8_t *data, uint8_t datasize) {
                BTHIDs::sendReport(data, datasize);
        };

        /**@}*/


private:

};
#endif
