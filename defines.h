#pragma once

#define extend(c)   (c | 0x80)

/*  Scancodes  */
enum : uint8_t {
    // alphabets
    SC_A    = 0x1C,
    SC_B    = 0x32,
    SC_C    = 0x21,
    SC_D    = 0x23,
    SC_E    = 0x24,
    SC_F    = 0x2B,
    SC_G    = 0x34,
    SC_H    = 0x33,
    SC_I    = 0x43,
    SC_J    = 0x3B,
    SC_K    = 0x42,
    SC_L    = 0x4B,
    SC_M    = 0x3A,
    SC_N    = 0x31,
    SC_O    = 0x44,
    SC_P    = 0x4D,
    SC_Q    = 0x15,
    SC_R    = 0x2D,
    SC_S    = 0x1B,
    SC_T    = 0x2C,
    SC_U    = 0x3C,
    SC_V    = 0x2A,
    SC_W    = 0x1D,
    SC_X    = 0x22,
    SC_Y    = 0x35,
    SC_Z    = 0x1A,
    // digits
    SC_1    = 0x16, // 1 !
    SC_2    = 0x1E, // 2 @
    SC_3    = 0x26, // 3 #
    SC_4    = 0x25, // 4 $
    SC_5    = 0x2E, // 5 %
    SC_6    = 0x36, // 6 ^
    SC_7    = 0x3D, // 7 &
    SC_8    = 0x3E, // 8 *
    SC_9    = 0x46, // 9 (
    SC_0    = 0x45, // 0 )
    // symbols
    SC_GRAVE= 0x0E, // ` ~
    SC_MINUS= 0x4E, // - _
    SC_EQUAL= 0x55, // = +
    SC_BSLDS= 0x5D, // \ |
    SC_BR_L = 0x54, // [ {
    SC_BR_R = 0x5B, // ] }
    SC_COLON= 0x4C, // ; :
    SC_APOS = 0x52, // ' "
    SC_COMMA= 0x41, // , <
    SC_DOT  = 0x49, // . >
    SC_SLDS = 0x4A, // / ?
    SC_SPACE= 0x29,
    // function keys
    SC_F1   = 0x05,
    SC_F2   = 0x06,
    SC_F3   = 0x04,
    SC_F4   = 0x0C,
    SC_F5   = 0x03,
    SC_F6   = 0x0B,
    SC_F7   = 0x83,
    SC_F8   = 0x0A,
    SC_F9   = 0x01,
    SC_F10  = 0x09,
    // modifiers
    SC_SHIFT= 0x12,
    SC_CTRL = 0x14,
    SC_ALT  = 0x11,
    // others
    SC_ESC  = 0x76,
    SC_TAB  = 0x0D,
    SC_CAPS = 0x58,
    SC_BS   = 0x66,
    SC_ENTER= 0x5A,
    SC_INS  = extend(0x70),
    SC_DEL  = extend(0x71),
    SC_HOME = extend(0x6C),
    SC_END  = extend(0x69),
    SC_PGUP = extend(0x7D),
    SC_PGDN = extend(0x7A),
    SC_UP   = extend(0x75),
    SC_DOWN = extend(0x72),
    SC_LEFT = extend(0x6B),
    SC_RIGHT= extend(0x74),
    SC_KANA = 0x13,
    // protocol
    SC_PRE  = 0xE0,
    SC_BREAK= 0xF0
};

/*  Comman from host  */
enum : uint8_t {
    REQ_STATUS_LED  = 0xED,
    REQ_ECHO        = 0xEE,
    REQ_SCANCODE_SET= 0xF0,
    REQ_DEVICE_ID   = 0xF2,
    REQ_REPEAT_RATE = 0xF3,
    REQ_ENABLE      = 0xF4,
    REQ_DISABLE     = 0xF5,
    REQ_RESEND      = 0xFE,
    REQ_RESET       = 0xFF,
};

/*  Response to host  */
enum : uint8_t {
    RESP_ACK        = 0xFA,
    RESP_ECHO       = 0xEE,
    RESP_DEVICE_ID1 = 0xAB,
    RESP_DEVICE_ID2 = 0x83,
};
