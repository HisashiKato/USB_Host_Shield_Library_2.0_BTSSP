#include "xboxonebtparser.h"

JoystickReportParser::JoystickReportParser(JoystickEvents *evt) :
joyEvents(evt),
oldHat(0x00),
oldButtons(0) {
        for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++)
                oldPad[i] = 0x8000;
};


void JoystickReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
        bool match = true;

        uint16_t pad[RPT_GAMEPAD_LEN];
        for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++) {
            pad[i] = (0x0000 | buf[(i * 2) + 1]);
            pad[i] <<= 8;
            pad[i] |= buf[(i * 2)];
	    }
	
		
		// Checking if there are changes in report since the method was last called
        for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++) {
                if (pad[i] != oldPad[i]) {
                        match = false;
                        break;
                }
        }
				
        // Calling Game Pad event handler
        if (!match && joyEvents) {
                joyEvents->OnGamePadChanged((const GamePadEventData*)buf);

                for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++) {
					oldPad[i] = pad[i];
                	padData[i] = oldPad[i];
				}
		}

        uint8_t hat = buf[12];

        // Calling Hat Switch event handler
        if (hat != oldHat && joyEvents) {
                joyEvents->OnHatSwitch(hat);
                oldHat = hat;
                hatData = oldHat;
		}



        uint16_t buttons = (0x0000 | (((B00000001 & buf[15]) << 2) | ((B00011000 & buf[14]) >> 3)));
        buttons <<= 8;
        buttons |= (((B01100000 & buf[14]) << 1) | ((B11000000 & buf[13]) >> 2) | ((B00011000 & buf[13]) >> 1) | (B00000011 & buf[13]));
        uint16_t changes = (buttons ^ oldButtons);


        // Calling Button Event Handler for every button changed
        if (changes) {
                for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
                        uint16_t mask = (0x0001 << i);

                        if (((mask & changes) > 0) && joyEvents) {
                                if ((buttons & mask) > 0) {
                                        joyEvents->OnButtonDn(i + 1);
                                        buttonsData[i] = 1;
								} else {
                                        joyEvents->OnButtonUp(i + 1);
                                        buttonsData[i] = 0;
								}
                        }
				}
                oldButtons = buttons;
        }
}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
///*
        Serial.print("   X: ");
        PrintHex<uint16_t > (evt->X, 0x80);
        Serial.print("  Y: ");
        PrintHex<uint16_t > (evt->Y, 0x80);
        Serial.print("   Z1: ");
        PrintHex<uint16_t > (evt->Z1, 0x80);
        Serial.print("  Z2: ");
        PrintHex<uint16_t > (evt->Z2, 0x80);
        Serial.print("   LT: ");
        PrintHex<uint16_t > (evt->LT, 0x80);
        Serial.print("   RT: ");
        PrintHex<uint16_t > (evt->RT, 0x80);
        Serial.println("");
//*/
}

void JoystickEvents::OnHatSwitch(uint8_t hat) {
///*
        Serial.print("Hat Switch: ");
        PrintHex<uint8_t > (hat, 0x80);
        Serial.println("");
//*/
}

void JoystickEvents::OnButtonUp(uint8_t but_id) {
///*
        Serial.print("Up: ");
        Serial.println(but_id, DEC);
//*/
}

void JoystickEvents::OnButtonDn(uint8_t but_id) {
///*
        Serial.print("Dn: ");
        Serial.println(but_id, DEC);
//*/
}
