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

#ifndef _swprobtparser_h_
#define _swprobtparser_h_

#include "Usb.h"
#include "BTHIDs.h"

enum JoystickEnum {
        STICK_LX, //X_Axis     // left_stick_x
        STICK_LY, //Y_Axis     // left_stick_y
        STICK_RX, //X_Rotation // right_stick_x
        STICK_RY  //Y_Rotation // right_stick_y
};

enum DpadEnum {
        /** Pro Controller Dpad Enum */
        DPAD_UP = 0x00,
        DPAD_UP_RIGHT = 0x01,
        DPAD_RIGHT = 0x02,
        DPAD_RIGHT_DOWN = 0x03,
        DPAD_DOWN = 0x04,
        DPAD_DOWN_LEFT = 0x05,
        DPAD_LEFT = 0x06,
        DPAD_LEFT_UP = 0x07,
        DPAD_NOT_PRESSED = 0x08
};

enum ButtonEnum {
        /** Pro Controller buttons Enum */
        B,
        A,
        Y,
        X,
        LB,
        RB,
        ZL,
        ZR,
        MINUS,
        PLUS,
        LS,
        RS,
        HOME,
        CAPTURE
};




const uint16_t BUTTONMASK[] PROGMEM = {
        /* Nintendo Switch Pro Controller */
                       //buf  1  2
        0x0001, // B       //01 00
        0x0002, // A       //02 00
        0x0004, // Y       //04 00
        0x0008, // X       //08 00

        0x0010, // LB      //10 00
        0x0020, // RB      //20 00
        0x0040, // ZL      //40 00
        0x0080, // ZR      //80 00

        0x0100, // MINUS   //00 01
        0x0200, // PLUS    //00 02
        0x0400, // LS      //00 04
        0x0800, // RS      //00 08

        0x1000, // HOME    //00 10
        0x2000  // CAPTURE //00 20
};


union SWProBTButtons {
        /* Nintendo Switch Pro Controller */
        struct {
                // buf 1
                uint8_t b : 1;
                uint8_t a : 1;
                uint8_t y : 1;
                uint8_t x : 1;
                uint8_t lb : 1;
                uint8_t rb : 1;
                uint8_t zl : 1;
                uint8_t zr : 1;

                // buf 2
                uint8_t minus : 1;
                uint8_t plus : 1;
                uint8_t ls : 1;
                uint8_t rs : 1;
                uint8_t home : 1;
                uint8_t capture : 1;
                uint8_t dummy_1 : 2; //Not used in Pro Controller
        } __attribute__((packed));

        uint16_t value : 16;

} __attribute__((packed));


struct SWProBTReceivedData {
        /* Nintendo Switch Pro Controller */
        SWProBTButtons btn;   // buf 1 2
        uint8_t dpad;         // buf 3
        uint16_t joystick[4]; // buf 4 - 11
} __attribute__((packed));

//-----------------------------------

//#define BT_HIDP_SW_SUBCMD_ACK 0x21
struct SWProBTReceivedSubCommandAck {
        uint8_t replyData[13];
        uint8_t replySubCommand;
        uint8_t replySubCommandData[34];
} __attribute__((packed));




struct SWProBTSendConfigData {
        uint8_t gpnum;
        uint8_t rumbleDataL[4];
        uint8_t rumbleDataR[4];
        uint8_t subCommand;
        uint8_t subCommandData[38];
} __attribute__((packed));


// The nameric value of rumble is based on the project of "BlueRetro".
// https://hackaday.io/project/170365-blueretro
//static const uint8_t sw_rumble_on[] = {0x28, 0x88, 0x60, 0x61, 0x28, 0x88, 0x60, 0x61};
//static const uint8_t sw_rumble_off[] = {0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40};

const uint8_t sw_rumble_on[4] = {0x28, 0x88, 0x60, 0x61};
const uint8_t sw_rumble_off[4] = {0x00, 0x01, 0x40, 0x40};


//-----------------------------------



/** This class parses all the data sent by the SWProBT controller */
class SWProBTParser {
public:
        /** Constructor for the SWProParser class. */
        SWProBTParser() {
                Reset();
        };

        /** @name SWProBT Controller functions */
        /**
         * Used to read the analog joystick.
         * @param  a  ::STICK_LX, ::STICK_LY, ::STICK_RX, and ::STICK_RY.
         * @return    Return the analog value in the range of 0-65535.
         */
        uint16_t stickData(JoystickEnum a);


        uint8_t dpadData();

        bool dpadClick(DpadEnum a);

        /**
         * buttonData(ButtonEnum b) will return true as long as the button is held down.
         *
         * While buttonClick(ButtonEnum b) will only return it once.
         *
         * So you instance if you need to increase a variable once you would use buttonClick(ButtonEnum b),
         * but if you need to drive a robot forward you would use buttonData(ButtonEnum b).
         * @param  b          ::ButtonEnum to read.
         * @return            buttonData(ButtonEnum b) will return a true as long as a button is held down, while buttonClick(ButtonEnum b) will return true once for each button press.
         */
        bool buttonData(ButtonEnum b);
        bool buttonClick(ButtonEnum b);



        void simpleRumbleL(bool l);
        void simpleRumbleR(bool r);


        /**@}*/

protected:
        /**
         * Used to parse data sent from the SWPro controller.
         * @param len Length of the data.
         * @param buf Pointer to the data buffer.
         */
        void Parse(uint8_t len, uint8_t *buf);

        /** Used to reset the different buffers to their default values */
        void Reset();

        void Init();

        virtual void sendReport(uint8_t *data, uint8_t datasize);


private:
        void sendConfigData(SWProBTSendConfigData *sendData);
        SWProBTReceivedData swprobtReceivedData;
        SWProBTButtons oldButtonState, buttonClickState;
        uint8_t pressedDpad;

        SWProBTSendConfigData swprobtSendConfigData;
        SWProBTReceivedSubCommandAck swprobtReceivedSubCommandAck;
        uint8_t sw_gpnum = 0;

        void setPlayerLED(uint8_t led);
        void setSimpleRumble();
        bool rumbleL = 0, rumbleR = 0;
        bool oldRumbleL = 0, oldRumbleR = 0;

};
#endif
