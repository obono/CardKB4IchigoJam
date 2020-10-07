//
// CardKB4IchigoJam.ino
// PS/2キーボード IFバージョン
//
// 必要ライブラリ
//  Adafruit_NeoPixel 
//
// 2019/03/10 PS/2イニシャル処理の修正（IchigoLatte対応、0x00は送信しない）
// 2019/03/10 起動直後のNeoPixcel点滅を廃止
// 2020/10/07 たま吉さんバージョン https://github.com/Tamakichi/CardKeyBoard_PS2
//            を基にプロジェクトを新規作成
//

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "ps2dev.h"
#include "maps.h"

#define KBD_PIN_CLK     A4  // PS/2 CLK  IchigoJam の KBD1 に接続
#define KBD_PIN_DATA    A5  // PS/2 DATA IchigoJam の KBD2 に接続

#define NUM_PIXELS      1
#define PIXEL_PIN       13

#define RESP_ACK        0xFA
#define RESP_ECHO       0xEE
#define RESP_DEVICE_ID1 0xAB
#define RESP_DEVICE_ID2 0x83

#define SCANCODE_SHIFT  0x59
#define SCANCODE_PREFIX 0xE0
#define SCANCODE_BREAK  0xF0

#define DELAY_SETUP     1000
#define DELAY_SCAN_ROW  2
#define DELAY_LOOP      10

static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static PS2dev   keyboard(KBD_PIN_CLK, KBD_PIN_DATA);
static uint8_t  idle = 0;
static uint8_t  modifierMode = 0;
static bool     isEnabled = false;
static bool     isShift = false;
static bool     isSym = false;
static bool     isFn = false;
static bool     isModifierLocked = false;

/*---------------------------------------------------------------------------*/

static uint16_t scanRow(uint8_t pin)
{
    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);
    digitalWrite(A1, HIGH);
    digitalWrite(A0, HIGH);
    digitalWrite(pin, LOW);
    delay(DELAY_SCAN_ROW);
    return ~(PIND | PINB << 8);
}

static void controlModifier(bool *pModifier, uint8_t mode)
{
    bool f = *pModifier;
    isShift = isSym = isFn = false;
    if (f) {
        *pModifier = !isModifierLocked;
        isModifierLocked = !isModifierLocked;
    } else {
        *pModifier = true;
        isModifierLocked = false;
    }
    modifierMode = (*pModifier) ? mode : MODE_NONE;
}

static int16_t findScanCodeMap(uint8_t keyCode)
{
    for (int16_t idx = 0; idx < sizeof(scanCodeMap) / 3; idx++) {
        if (pgm_read_byte(&scanCodeMap[idx][0]) == keyCode) {
            return idx;
        }
    }
    return -1;
}

static void sendAck()
{
    while (keyboard.write(RESP_ACK)) { ; }
}

static int processHostCommand(uint8_t cmd)
{
    uint8_t val;
    switch (cmd) {
    case 0xFF: // Reset: キーボードリセットコマンド。正しく受け取った場合ACKを返す。
        sendAck();
        break;
    case 0xFE: // 再送要求
        sendAck();
        break;
    case 0xF6: // 起動時の状態へ戻す
        sendAck();
        break;
    case 0xF5: //起動時の状態へ戻し、キースキャンを停止する
        isEnabled = false;
        sendAck();
        break;
    case 0xF4: //キースキャンを開始する
        isEnabled = true;
        sendAck();
        break;
    case 0xF3: //set typematic rate/delay :
        sendAck();
        keyboard.read(&val); //do nothing with the rate
        sendAck();
        break;
    case 0xF2: //get device id :
        sendAck();
        keyboard.write(RESP_DEVICE_ID1);
        keyboard.write(RESP_DEVICE_ID2);
        break;
    case 0xF0: //set scan scanCode set
        sendAck();
        keyboard.read(&val); //do nothing with the rate
        sendAck();
        break;
    case 0xEE: //echo :キーボードが接続されている場合、キーボードはパソコンへ応答（ECHO Responce）を返す。
        keyboard.write(RESP_ECHO);
        break;
    case 0xED: //set/reset LEDs :キーボードのLEDの点灯/消灯要求。これに続くオプションバイトでLEDを指定する。
        sendAck();
        keyboard.read(&val); //do nothing with the rate
        sendAck();
        break;
    default:
        break;
    }
}

static void sendScanCode(int16_t idx)
{
    uint8_t modify = pgm_read_byte(&scanCodeMap[idx][1]);
    uint8_t scanCode = pgm_read_byte(&scanCodeMap[idx][2]);

    /*  Send a make scanCode  */
    if (modify & 1) keyboard.write(SCANCODE_SHIFT);
    if (modify & 2) keyboard.write(SCANCODE_PREFIX);
    keyboard.write(scanCode);

    /*  Send a break scanCode  */
    if (modify & 2) keyboard.write(SCANCODE_PREFIX);
    keyboard.write(SCANCODE_BREAK);
    keyboard.write(scanCode);
    if (modify & 1) {
        keyboard.write(SCANCODE_BREAK);
        keyboard.write(SCANCODE_SHIFT);
    }
}

/*---------------------------------------------------------------------------*/

void setup()
{
    pinMode(A3, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(A0, OUTPUT);
    scanRow(A0);
    DDRB = 0x00;
    PORTB = 0xFF;
    DDRD = 0x00;
    PORTD = 0xFF;

    pixels.begin();

    uint32_t tm = millis() + 1000;
    while (keyboard.write(0xAA) != 0 && millis() <= tm) { ; }
}

void loop(void)
{
    uint16_t        keyState[KBD_ROWS];
    static uint16_t lastKeyState[KBD_ROWS] = { 0, 0, 0, 0 };

    if (digitalRead(KBD_PIN_CLK) == LOW || digitalRead(KBD_PIN_DATA) == LOW) {
        uint8_t cmd;
        while (keyboard.read(&cmd)) { ; }
        processHostCommand(cmd);
    }

    /*if (!isEnabled) {
        delay(DELAY_LOOP);
        return;
    }*/

    keyState[0] = scanRow(A3);
    keyState[1] = scanRow(A2);
    keyState[2] = scanRow(A1);
    keyState[3] = scanRow(A0);

    if ((keyState[0] & BIT_SHIFT) && !(lastKeyState[0] & BIT_SHIFT)) {
        controlModifier(&isShift, MODE_SHIFT);
    }
    if ((keyState[0] & BIT_SYM) && !(lastKeyState[0] & BIT_SYM)) {
        controlModifier(&isSym, MODE_SYM);
    }
    if ((keyState[0] & BIT_FN) && !(lastKeyState[0] & BIT_FN)) {
        controlModifier(&isFn, MODE_FN);
    }

    bool isPressed = false;
    for (int row = 0; row < KBD_ROWS; row++) {
        uint16_t state = keyState[row];
        uint16_t lastState = lastKeyState[row];
        for (int column = 0; column < KBD_COLUMNS; column++) {
            uint16_t b = 1 << column;
            if ((state & b) && !(lastState & b)) {
                uint8_t key = row * KBD_COLUMNS + column;
                uint8_t keyCode = pgm_read_byte(&keyCodeMap[key][modifierMode]);
                if (keyCode > 0) {
                    int16_t idx = findScanCodeMap(keyCode);
                    if (idx >= 0) {
                        sendScanCode(idx);
                        isPressed = true;
                    }
                }
            }
        }
    }

    if (!isModifierLocked && isPressed) {
        isShift = isSym = isFn = false;
        isModifierLocked = false;
        modifierMode = MODE_NONE;
    }
    if (isModifierLocked || (idle & 8)) {
        pixels.setPixelColor(0, pixels.Color(isShift * 5, isSym * 5, isFn * 5));
    } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    }
    pixels.show();

    memcpy(lastKeyState, keyState, sizeof(lastKeyState));
    idle++;
    delay(DELAY_LOOP);
}
