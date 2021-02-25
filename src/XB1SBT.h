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

#ifndef _xb1sbt_h_
#define _xb1sbt_h_

#include "BTHIDs.h"
#include "XB1SBTParser.h"
#include "XB1SBTRumble.h"

/**
 * This class implements support for the XB1S controller via Bluetooth.
 * It uses the BTHID class for all the Bluetooth communication.
 */
class XB1SBT : public BTHIDs, public XB1SBTParser, public XB1SBTRumble {
public:
        /**
         * Constructor for the XB1SBT class.
         * @param  p     Pointer to the BTDSSP class instance.
         * @param  pair  Set this to true in order to pair with the device. If the argument is omitted then it will not pair with it. One can use ::PAIR to set it to true.
         */
        XB1SBT(BTDSSP *p, bool pair = false) :
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
        /** @name BTHID implementation */
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
//                enable_sixaxis(); // Make the controller send out the entire output report
//                if (!pFuncOnInit)
//                        setLed(Blue); // Only call this is a user function has not been set
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
//        virtual void sendOutputReport(uint8_t id, uint8_t *data, uint8_t datasize);
/*
        virtual void sendOutputReport(XB1SOutput *output) { // Source: https://github.com/chrippa/ds4drv
                uint8_t buf[79];
                memset(buf, 0, sizeof(buf));

                buf[0] = 0x52; // HID BT Set_report (0x50) | Report Type (Output 0x02)
                buf[1] = 0x11; // Report ID
                buf[2] = 0x80;
                buf[4]= 0xFF;

                buf[7] = output->smallRumble; // Small Rumble
                buf[8] = output->bigRumble; // Big rumble

                buf[9] = output->r; // Red
                buf[10] = output->g; // Green
                buf[11] = output->b; // Blue

                buf[12] = output->flashOn; // Time to flash bright (255 = 2.5 seconds)
                buf[13] = output->flashOff; // Time to flash dark (255 = 2.5 seconds)

                output->reportChanged = false;

                // The PS4 console actually set the four last bytes to a CRC32 checksum, but it seems like it is actually not needed

                HID_Command(buf, sizeof(buf));
        };
*/
        /**@}*/

private:
/*
        void enable_sixaxis() { // Command used to make the PS4 controller send out the entire output report
                uint8_t buf[2];
                buf[0] = 0x43; // HID BT Get_report (0x40) | Report Type (Feature 0x03)
                buf[1] = 0x02; // Report ID

                HID_Command(buf, 2);
        };
        void HID_Command(uint8_t *data, uint8_t nbytes) {
                pBtdssp->L2CAP_Command(hci_handle, data, nbytes, control_scid[0], control_scid[1]);
        };
*/
};
#endif
