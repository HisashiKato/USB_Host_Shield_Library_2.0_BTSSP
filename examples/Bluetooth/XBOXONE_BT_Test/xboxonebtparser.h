#ifndef __XBOXONEBTPARSER_H__
#define __XBOXONEBTPARSER_H__

#include <usbhid.h>

struct GamePadEventData {
        uint16_t X, Y, Z1, Z2, LT ,RT;
};

class JoystickEvents {
public:
        virtual void OnGamePadChanged(const GamePadEventData *evt);
        virtual void OnHatSwitch(uint8_t hat);
        virtual void OnButtonUp(uint8_t but_id);
        virtual void OnButtonDn(uint8_t but_id);
};

#define RPT_GAMEPAD_LEN		6
#define NUM_BUTTONS        16

class JoystickReportParser : public HIDReportParser {
        JoystickEvents *joyEvents;

        uint16_t oldPad[RPT_GAMEPAD_LEN];
        uint8_t oldHat;
        uint16_t oldButtons;

public:
        uint16_t padData[RPT_GAMEPAD_LEN];
        uint8_t hatData;
        uint8_t buttonsData[NUM_BUTTONS];

        JoystickReportParser(JoystickEvents *evt);

        virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

#endif // __XBOXONEBTPARSER_H__
