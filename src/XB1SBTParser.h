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

#ifndef _xb1sbtparser_h_
#define _xb1sbtparser_h_

#include "Usb.h"


enum XB1S_JoystickEnum {
        STICK_LX, //X_Axis     // left_stick_x
        STICK_LY, //Y_Axis     // left_stick_y
        STICK_RX, //X_Rotation // right_stick_x
        STICK_RY  //Y_Rotation // right_stick_y
};

enum XB1S_TriggerEnum {
        LT, //Z_Axis     //trigger_left
        RT  //Z_Rotation //trigger_right
};

enum XB1S_DpadEnum {
        /** Xbox One S Dpad Enum */
        DPAD_NOT_PRESSED = 0x00,
        DPAD_UP = 0x01,
        DPAD_UP_RIGHT = 0x02,
        DPAD_RIGHT = 0x03,
        DPAD_RIGHT_DOWN = 0x04,
        DPAD_DOWN = 0x05,
        DPAD_DOWN_LEFT = 0x06,
        DPAD_LEFT = 0x07,
        DPAD_LEFT_UP = 0x08
};

enum XB1S_ButtonEnum {
        /** Xbox One S buttons Enum */
        A,
        B,
        X,
        Y,
        LB,
        RB,
        VIEW,
        MENU,
        LS,
        RS,
        XBOX
 };


const uint32_t XB1SBTBUTTONMASK[] PROGMEM = {
        /* Xbox One S Wireless (MODEL 1708 4.8.1923.0 via Bluetooth) */
                      //buf 14 15 16
        0x000001, // A    //01 00 00
        0x000002, // B    //02 00 00
        0x000008, // X    //08 00 00
        0x000010, // Y    //10 00 00

        0x000040, // LB   //40 00 00
        0x000080, // RB   //80 00 00

        0x010000, // VIEW //00 00 01
        0x000800, // MENU //00 08 00

        0x002000, // LS   //00 20 00
        0x004000, // RS   //00 40 00

        0x001000  // XBOX //00 10 00
};


union XB1SBTButtons {
        /* Xbox One S Wireless (MODEL 1708 4.8.1923.0 via Bluetooth) */
        struct {
                // buf 14
                uint8_t a : 1;
                uint8_t b : 1;
                uint8_t dummy_1 : 1;
                uint8_t x : 1;
                uint8_t y : 1;
                uint8_t dummy_2 : 1;
                uint8_t lb : 1;
                uint8_t rb : 1;

                // buf 15
                uint8_t dummy_3 : 3;
                uint8_t menu : 1;
                uint8_t xbox : 1;
                uint8_t ls : 1;
                uint8_t rs : 1;
                uint8_t dummy_4 : 1;

                // buf 16
                uint8_t view : 1;
                uint8_t dummy_5 : 7;
        } __attribute__((packed));

        uint32_t value : 24;

} __attribute__((packed));


struct XB1SBTReceivedData {
        /* Xbox One S Wireless (MODEL 1708 4.8.1923.0 via Bluetooth) */
        uint16_t joystick[4]; // buf  1 -  8
        uint16_t trigger[2];  // buf  9 - 12
        uint8_t dpad;         // buf 13
        XB1SBTButtons btn;    // buf 14 - 16 
} __attribute__((packed));


/*
I used it as a reference. xpadneo:https://github.com/atar-axis/xpadneo
https://github.com/atar-axis/xpadneo/blob/master/docs/reports/xb1s_battery_event.md
*/

union XB1SBTBatteryStatus {
        struct {
                uint8_t batterylevel : 2;
                uint8_t powersupply : 2;
                uint8_t charging : 1;
                uint8_t error : 1;
                uint8_t reserved : 1;
                uint8_t online : 1;
        } __attribute__((packed));

        uint8_t data;

} __attribute__((packed));




/** This class parses all the data sent by the XB1SBT controller */
class XB1SBTParser {
public:
        /** Constructor for the PS4Parser class. */
        XB1SBTParser() {
                Reset();
        };

        /** @name XB1SBT Controller functions */
        /**
         * Used to read the analog joystick.
         * @param  a  ::STICK_LX, ::STICK_LY, ::STICK_RX, and ::STICK_RY.
         * @return    Return the analog value in the range of 0-65535.
         */
        uint16_t stickData(XB1S_JoystickEnum a);

        /**
         * Used to get the analog value from Trigger.
         * @param  b  The ::TriggerEnum ::LT and RT.
         * @return    Return the analog value in the range of 0-1023.
         */
        uint16_t triggerData(XB1S_TriggerEnum a);


        uint8_t dpadData();

        bool dpadClick(XB1S_DpadEnum a);

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
        bool buttonData(XB1S_ButtonEnum b);
        bool buttonClick(XB1S_ButtonEnum b);



        /**
         * Return the battery level of the XB1S controller.
         * @return The battery level in the range 0-3.
         */
        uint8_t batteryLevel();

        uint8_t batteryStatus();



        /**@}*/

protected:
        /**
         * Used to parse data sent from the XB1S controller.
         * @param len Length of the data.
         * @param buf Pointer to the data buffer.
         */
        void Parse(uint8_t len, uint8_t *buf);

        /** Used to reset the different buffers to their default values */
        void Reset();


private:
        XB1SBTReceivedData xb1sbtReceivedData;
        XB1SBTButtons oldButtonState, buttonClickState;
        uint8_t pressedDpad;
        XB1SBTBatteryStatus xb1sbtBatteryStatus;
};
#endif
