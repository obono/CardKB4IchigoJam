/**
 * CardKeyBoard for IchigoJam (US-layout)
 * 
 * Based on CardKeyBoard_PS2 by Tamakichi
 * https://github.com/Tamakichi/CardKeyBoard_PS2
 * 
 * Dependency: Adafruit_NeoPixel
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "ps2dev.h"
#include "defines.h"
#include "layout.h"

#define VERSION         "1.0.0"

#define NUM_PIXELS      1
#define PIXEL_PIN       13
#define PIXEL_VALUE     4

#define DELAY_SETUP     1000
#define DELAY_SCAN_ROW  2
#define DELAY_ALT_MODE  500
#define DELAY_LOOP      10

enum : uint8_t {
    MODE_NONE = 0,
    MODE_SYM,
    MODE_FN,
    MODE_ALT,
    MODE_MAX,
};

enum : uint8_t {
    SHIFT_MODE_NONE = 0,
    SHIFT_MODE_ONCE,
    SHIFT_MODE_LOCKED,
    SHIFT_MODE_MAX,
};

static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static PS2dev   keyboard(KBD_PIN_CLK, KBD_PIN_DATA);
static uint32_t altModeTime, keyRepeatTime;
static uint16_t delayKeyRepeat = 500, intervalKeyRepeat = 100;
static uint8_t  mode = MODE_NONE, shiftMode = SHIFT_MODE_NONE;
static int8_t   lastKey = -1, lastScancode = 0;
static bool     isEnabled = false, isModeLocked = false, isModifierOn = false;

/*---------------------------------------------------------------------------*/

static uint16_t scanKeyboardRow(uint8_t pin)
{
    for (int row = 0; row < KBD_ROWS; row++) {
        uint8_t rowPin = pgm_read_byte(&keyboardRowPins[row]);
        digitalWrite(rowPin, (pin == rowPin) ? LOW : HIGH);
    }
    delay(DELAY_SCAN_ROW);
    return ~(PIND | PINB << 8);
}

static void sendScancode(uint8_t scancode, bool isBreak)
{
    if (scancode > SC_F7) {
        if (scancode >= SC_END) keyboard.write(SC_PRE);
        scancode &= 0x7F;
    }
    if (isBreak) keyboard.write(SC_BREAK);
    keyboard.write(scancode);
}

static void clearShiftMode(void)
{
    if (shiftMode != SHIFT_MODE_NONE) sendScancode(SC_SHIFT, true);
    shiftMode = SHIFT_MODE_NONE;
}

static void controlMode(uint8_t newMode)
{
    if (mode != newMode || isModeLocked) {
        if (mode == MODE_SYM && isModifierOn) sendScancode(SC_SHIFT, true);
        if (mode == MODE_FN  && isModifierOn) sendScancode(SC_CTRL, true);
        if (mode == MODE_ALT && isModifierOn) sendScancode(SC_ALT, true);
        if (mode == newMode) newMode = MODE_NONE;
        isModeLocked = false;
        isModifierOn = false;
    }
    if (mode != newMode) {
        if (newMode == MODE_SYM) clearShiftMode();
    } else {
        isModeLocked = true;
    }
    mode = newMode;
}

static void toggleShiftMode(void)
{
    if (mode == MODE_SYM) controlMode(MODE_NONE);
    if (shiftMode == SHIFT_MODE_NONE)   sendScancode(SC_SHIFT, false);
    if (shiftMode == SHIFT_MODE_LOCKED) sendScancode(SC_SHIFT, true);
    shiftMode = (shiftMode + 1) % SHIFT_MODE_MAX;
}

static void controlModifier(bool isToModify, uint8_t scancode) {
    if (isToModify != isModifierOn) {
        sendScancode(scancode, !isToModify);
        isModifierOn = isToModify;
    }
}

static void sendAck()
{
    while (keyboard.write(RESP_ACK)) { ; }
}

static int processHostCommand(uint8_t cmd)
{
    uint8_t val;
    switch (cmd) {
    case REQ_STATUS_LED:
    case REQ_SCANCODE_SET:
        sendAck();
        keyboard.read(&val); // read but do nothing
        sendAck();
        break;
    case REQ_ECHO:
        keyboard.write(RESP_ECHO);
        break;
    case REQ_DEVICE_ID:
        sendAck();
        keyboard.write(RESP_DEVICE_ID1);
        keyboard.write(RESP_DEVICE_ID2);
        break;
    case REQ_REPEAT_RATE:
        sendAck();
        keyboard.read(&val);
        sendAck();
        delayKeyRepeat = ((val >> 5 & 0x3) + 1) * 250;
        intervalKeyRepeat = 25 * (((val & 0x7) + 8) << (val >> 3 & 0x3)) / 6;
        break;
    case REQ_ENABLE:
    case REQ_DISABLE:
        isEnabled = (cmd == REQ_ENABLE);
        // go to following line
    case REQ_RESEND:
    case REQ_RESET:
    default:
        sendAck();
        break;
    }
}

static void keyPressed(uint8_t key)
{
    uint8_t idx = (mode == MODE_ALT) ? MODE_NONE : mode;
    uint8_t scancode = pgm_read_byte(&scancodeMap[key][idx]);
    if (!scancode) return;
    if (lastKey != -1) keyReleased(lastKey);
    if (mode == MODE_SYM) controlModifier(isShiftedCode(scancode), SC_SHIFT);
    if (mode == MODE_FN)  controlModifier(isModifyableCode(scancode), SC_CTRL);
    if (mode == MODE_ALT) controlModifier(isModifyableCode(scancode), SC_ALT);
    sendScancode(scancode, false);
    lastKey = key;
    lastScancode = scancode;
    keyRepeatTime = millis() + delayKeyRepeat;
}

static void keyReleased(uint8_t key)
{
    if (key != lastKey) return;
    sendScancode(lastScancode, true);
    lastKey = -1;
    lastScancode = 0;
    if (mode != MODE_NONE && !isModeLocked) controlMode(MODE_NONE);
    if (shiftMode == SHIFT_MODE_ONCE) clearShiftMode();
}

static void controlPixel(void)
{
    static uint8_t counter = 0;
    uint8_t r = 0, g = 0, b = 0;
    if (lastKey == -1) {
        bool isBlinkOn = (counter & 8);
        if (shiftMode == SHIFT_MODE_ONCE && isBlinkOn || shiftMode == SHIFT_MODE_LOCKED) {
            r = PIXEL_VALUE;
        }
        if (isBlinkOn || isModeLocked) {
            if (mode == MODE_SYM || mode == MODE_ALT) g = PIXEL_VALUE;
            if (mode == MODE_SYM || mode == MODE_FN)  b = PIXEL_VALUE;
        }
    } else {
        r = g = b = PIXEL_VALUE;
    }
    pixels.setPixelColor(0, pixels.Color(r, g, b));
    pixels.show();
    counter++;
}

/*---------------------------------------------------------------------------*/

void setup()
{
    for (int row = 0; row < KBD_ROWS; row++) {
        pinMode(pgm_read_byte(&keyboardRowPins[row]), OUTPUT);
    }
    DDRB = 0x00;
    PORTB = 0xFF;
    DDRD = 0x00;
    PORTD = 0xFF;
    pixels.begin();
    uint32_t targetTime = millis() + DELAY_SETUP;
    while (keyboard.write(0xAA) != 0 && millis() <= targetTime) { ; }
}

void loop(void)
{
    uint16_t        keyState[KBD_ROWS];
    static uint16_t lastKeyState[KBD_ROWS] = { 0, 0, 0, 0 };
    uint32_t        currentTime = millis();

    if (digitalRead(KBD_PIN_CLK) == LOW || digitalRead(KBD_PIN_DATA) == LOW) {
        uint8_t cmd;
        while (keyboard.read(&cmd)) { ; }
        processHostCommand(cmd);
    }

    for (int row = 0; row < KBD_ROWS; row++) {
        keyState[row] = scanKeyboardRow(pgm_read_byte(&keyboardRowPins[row]));
    }
    if ((keyState[0] & BIT_SHIFT) && !(lastKeyState[0] & BIT_SHIFT)) toggleShiftMode();
    if ((keyState[0] & BIT_FN) && !(lastKeyState[0] & BIT_FN)) controlMode(MODE_FN);
    if (keyState[0] & BIT_SYM) {
        if (!(lastKeyState[0] & BIT_SYM)) {
            controlMode((mode == MODE_ALT) ? MODE_ALT : MODE_SYM);
            if (mode == MODE_SYM && !isModeLocked) altModeTime = currentTime + DELAY_ALT_MODE;
        } else if (mode == MODE_SYM && !isModeLocked && currentTime >= altModeTime) {
            controlMode(MODE_ALT);
        }
    }

    for (int row = 0; row < KBD_ROWS; row++) {
        uint16_t state = keyState[row];
        uint16_t lastState = lastKeyState[row];
        for (int column = 0; column < KBD_COLUMNS; column++) {
            uint16_t b = bit(column);
            bool current = state & b;
            bool last = lastState & b;
            uint8_t key = row * KBD_COLUMNS + column;
            if (current && !last) keyPressed(key);
            if (!current && last) keyReleased(key);
        }
    }

    if (lastKey == -1) {
        if (isModifierOn) {
            if (mode == MODE_SYM) sendScancode(SC_SHIFT, true);
            if (mode == MODE_FN)  sendScancode(SC_CTRL, true);
            if (mode == MODE_ALT) sendScancode(SC_ALT, true);
            isModifierOn = false;
        }
    } else {
        if (currentTime >= keyRepeatTime) {
            sendScancode(lastScancode, false);
            keyRepeatTime += intervalKeyRepeat;
        }
    }

    controlPixel();
    memcpy(lastKeyState, keyState, sizeof(lastKeyState));
    delay(DELAY_LOOP);
}
