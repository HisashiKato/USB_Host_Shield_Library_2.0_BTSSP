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

#ifndef _xb1sbts_h_
#define _xb1sbts_h_

#include "BTHIDs.h"
#include "XB1SBTParser.h"

/**
 * This class implements support for the XB1S controller via Bluetooth.
 * It uses the BTHIDs class for all the Bluetooth communication.
 */
class XB1SBTS : public BTHIDs, public XB1SBTParser {
public:
        /**
         * Constructor for the XB1SBTS class.
         * @param  p     Pointer to the BTDSSP class instance.
         * @param  pair  Set this to true in order to pair with the device. If the argument is omitted then it will not pair with it. One can use ::PAIR to set it to true.
         */
        XB1SBTS(BTDSSP *p, bool pair = false) :
        BTHIDs(p, pair) {
                XB1SBTParser::Reset();
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
         */
        virtual void OnInitBTHIDs() {
                XB1SBTParser::Reset();
        };

        /** Used to reset the different buffers to there default values */
        virtual void ResetBTHIDs() {
                XB1SBTParser::Reset();
        };
        /**@}*/

private:

};
#endif
