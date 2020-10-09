#pragma once

#define KBD_PIN_CLK     A4
#define KBD_PIN_DATA    A5

//       d0   d1     d2  d3 d4 d5 d6 d7 d8 d9 d10 d11
//A3：   esc   1      2  3  4  5  6  7  8  9  0   del
//A2:    tab   q      w  e  r  t  y  u  i  o  p
//A1:   left   up     a  s  d  f  g  h  j  k  l   enter
//A0:   down   right  z  x  c  v  b  n  m  ,  .   space

#define KBD_ROWS        4
#define KBD_COLUMNS     12

#define KBD_PIN_ROW0    A3
#define KBD_PIN_ROW1    A2
#define KBD_PIN_ROW2    A1
#define KBD_PIN_ROW3    A0

#define BIT_SHIFT       bit(12) // shift: d12
#define BIT_SYM         bit(15) // sym: d15
#define BIT_FN          bit(14) // fn: d14

PROGMEM static const uint8_t keyboardRowPins[KBD_ROWS] = {
    KBD_PIN_ROW0, KBD_PIN_ROW1, KBD_PIN_ROW2, KBD_PIN_ROW3
};

#define shift(x)    (x | 0x80)

PROGMEM static const uint8_t scancodeMap[KBD_ROWS * KBD_COLUMNS][3] = {
    //  Normal,     Sym,            Fn
    {   SC_ESC,     SC_ESC,         SC_ESC  }, // Esc
    {   SC_1,       shift(SC_1),    SC_F1   }, // 1
    {   SC_2,       shift(SC_2),    SC_F2   }, // 2
    {   SC_3,       shift(SC_3),    SC_F3   }, // 3
    {   SC_4,       shift(SC_4),    SC_F4   }, // 4
    {   SC_5,       shift(SC_5),    SC_F5   }, // 5
    {   SC_6,       shift(SC_6),    SC_F6   }, // 6
    {   SC_7,       shift(SC_7),    SC_F7   }, // 7
    {   SC_8,       shift(SC_8),    SC_F8   }, // 8
    {   SC_9,       shift(SC_9),    SC_F9   }, // 9
    {   SC_0,       shift(SC_0),    SC_F10  }, // 0
    {   SC_BS,      SC_DEL,         SC_INS  }, // Delete
    {   SC_TAB,     SC_TAB,         SC_CAPS }, // Tab
    {   SC_Q,       shift(SC_BR_L), SC_Q    }, // Q
    {   SC_W,       shift(SC_BR_R), SC_W    }, // W
    {   SC_E,       SC_BR_L,        SC_E    }, // E
    {   SC_R,       SC_BR_R,        SC_R    }, // R
    {   SC_T,       SC_SLDS,        SC_T    }, // T
    {   SC_Y,       SC_BSLDS,       SC_Y    }, // Y
    {   SC_U,       shift(SC_BSLDS),SC_U    }, // U
    {   SC_I,       shift(SC_GRAVE),SC_I    }, // I
    {   SC_O,       SC_APOS,        SC_O    }, // O
    {   SC_P,       shift(SC_APOS), SC_P    }, // P
    {   0,          0,              0       }, // (no key)
    {   SC_LEFT,    SC_LEFT,        SC_HOME }, // Left
    {   SC_UP,      SC_UP,          SC_PGUP }, // Up
    {   SC_A,       SC_COLON,       SC_A    }, // A
    {   SC_S,       shift(SC_COLON),SC_S    }, // S
    {   SC_D,       SC_GRAVE,       SC_D    }, // D
    {   SC_F,       shift(SC_EQUAL),SC_F    }, // F
    {   SC_G,       SC_MINUS,       SC_G    }, // G
    {   SC_H,       shift(SC_MINUS),SC_H    }, // H
    {   SC_J,       SC_EQUAL,       SC_J    }, // J
    {   SC_K,       shift(SC_SLDS), SC_K    }, // K
    {   SC_L,       0,              SC_L    }, // L
    {   SC_ENTER,   SC_ENTER,       SC_ENTER}, // Enter
    {   SC_DOWN,    SC_DOWN,        SC_PGDN }, // Down
    {   SC_RIGHT,   SC_RIGHT,       SC_END  }, // Right
    {   SC_Z,       0,              SC_Z    }, // Z
    {   SC_X,       0,              SC_X    }, // X
    {   SC_C,       0,              SC_C    }, // C
    {   SC_V,       0,              SC_V    }, // V
    {   SC_B,       0,              SC_B    }, // B
    {   SC_N,       0,              SC_N    }, // N
    {   SC_M,       0,              SC_M    }, // M
    {   SC_COMMA,   shift(SC_COMMA),SC_COMMA}, // ,
    {   SC_DOT,     shift(SC_DOT),  SC_DOT  }, // .
    {   SC_SPACE,   SC_SPACE,       SC_KANA }  // Space
};

#define isShiftedCode(c)    (c >= shift(SC_GRAVE) && c <= shift(SC_BSLDS))
#define isModifyableCode(c) (c >= SC_Q && c <= SC_P)
