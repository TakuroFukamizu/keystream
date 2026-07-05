#include "keymap.h"

// USキーマップ定義
const KeyMapping keymap_us[128] = {
    // 0-31: 制御文字 (改行、タブなど)
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {HID_KEY_BACKSPACE, MOD_NONE}, {HID_KEY_TAB, MOD_NONE}, {HID_KEY_ENTER, MOD_NONE}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

    // 32-47: スペースおよび記号 ( , !, ", #, $, %, &, ', (, ), *, +, ,, -, ., / )
    {HID_KEY_SPACE, MOD_NONE},              // ' ' (32)
    {HID_KEY_1, MOD_SHIFT},                 // '!'
    {HID_KEY_APOSTROPHE, MOD_SHIFT},        // '"'
    {HID_KEY_3, MOD_SHIFT},                 // '#'
    {HID_KEY_4, MOD_SHIFT},                 // '$'
    {HID_KEY_5, MOD_SHIFT},                 // '%'
    {HID_KEY_7, MOD_SHIFT},                 // '&'
    {HID_KEY_APOSTROPHE, MOD_NONE},         // '\''
    {HID_KEY_9, MOD_SHIFT},                 // '('
    {HID_KEY_0, MOD_SHIFT},                 // ')'
    {HID_KEY_8, MOD_SHIFT},                 // '*'
    {HID_KEY_EQUAL, MOD_SHIFT},             // '+'
    {HID_KEY_COMMA, MOD_NONE},              // ','
    {HID_KEY_MINUS, MOD_NONE},              // '-'
    {HID_KEY_PERIOD, MOD_NONE},             // '.'
    {HID_KEY_SLASH, MOD_NONE},              // '/' (47)

    // 48-57: 数字 (0-9)
    {HID_KEY_0, MOD_NONE}, {HID_KEY_1, MOD_NONE}, {HID_KEY_2, MOD_NONE}, {HID_KEY_3, MOD_NONE}, {HID_KEY_4, MOD_NONE},
    {HID_KEY_5, MOD_NONE}, {HID_KEY_6, MOD_NONE}, {HID_KEY_7, MOD_NONE}, {HID_KEY_8, MOD_NONE}, {HID_KEY_9, MOD_NONE},

    // 58-64: 記号 ( :, ;, <, =, >, ?, @ )
    {HID_KEY_SEMICOLON, MOD_SHIFT},         // ':' (58)
    {HID_KEY_SEMICOLON, MOD_NONE},          // ';'
    {HID_KEY_COMMA, MOD_SHIFT},             // '<'
    {HID_KEY_EQUAL, MOD_NONE},              // '='
    {HID_KEY_PERIOD, MOD_SHIFT},            // '>'
    {HID_KEY_SLASH, MOD_SHIFT},             // '?'
    {HID_KEY_2, MOD_SHIFT},                 // '@' (64)

    // 65-90: 大文字 (A-Z)
    {HID_KEY_A, MOD_SHIFT}, {HID_KEY_B, MOD_SHIFT}, {HID_KEY_C, MOD_SHIFT}, {HID_KEY_D, MOD_SHIFT}, {HID_KEY_E, MOD_SHIFT},
    {HID_KEY_F, MOD_SHIFT}, {HID_KEY_G, MOD_SHIFT}, {HID_KEY_H, MOD_SHIFT}, {HID_KEY_I, MOD_SHIFT}, {HID_KEY_J, MOD_SHIFT},
    {HID_KEY_K, MOD_SHIFT}, {HID_KEY_L, MOD_SHIFT}, {HID_KEY_M, MOD_SHIFT}, {HID_KEY_N, MOD_SHIFT}, {HID_KEY_O, MOD_SHIFT},
    {HID_KEY_P, MOD_SHIFT}, {HID_KEY_Q, MOD_SHIFT}, {HID_KEY_R, MOD_SHIFT}, {HID_KEY_S, MOD_SHIFT}, {HID_KEY_T, MOD_SHIFT},
    {HID_KEY_U, MOD_SHIFT}, {HID_KEY_V, MOD_SHIFT}, {HID_KEY_W, MOD_SHIFT}, {HID_KEY_X, MOD_SHIFT}, {HID_KEY_Y, MOD_SHIFT},
    {HID_KEY_Z, MOD_SHIFT},

    // 91-96: 記号 ( [, \, ], ^, _, ` )
    {HID_KEY_BRACKET_LEFT, MOD_NONE},       // '[' (91)
    {HID_KEY_BACKSLASH, MOD_NONE},          // '\'
    {HID_KEY_BRACKET_RIGHT, MOD_NONE},      // ']'
    {HID_KEY_6, MOD_SHIFT},                 // '^'
    {HID_KEY_MINUS, MOD_SHIFT},             // '_'
    {HID_KEY_GRAVE, MOD_NONE},              // '`' (96)

    // 97-122: 小文字 (a-z)
    {HID_KEY_A, MOD_NONE}, {HID_KEY_B, MOD_NONE}, {HID_KEY_C, MOD_NONE}, {HID_KEY_D, MOD_NONE}, {HID_KEY_E, MOD_NONE},
    {HID_KEY_F, MOD_NONE}, {HID_KEY_G, MOD_NONE}, {HID_KEY_H, MOD_NONE}, {HID_KEY_I, MOD_NONE}, {HID_KEY_J, MOD_NONE},
    {HID_KEY_K, MOD_NONE}, {HID_KEY_L, MOD_NONE}, {HID_KEY_M, MOD_NONE}, {HID_KEY_N, MOD_NONE}, {HID_KEY_O, MOD_NONE},
    {HID_KEY_P, MOD_NONE}, {HID_KEY_Q, MOD_NONE}, {HID_KEY_R, MOD_NONE}, {HID_KEY_S, MOD_NONE}, {HID_KEY_T, MOD_NONE},
    {HID_KEY_U, MOD_NONE}, {HID_KEY_V, MOD_NONE}, {HID_KEY_W, MOD_NONE}, {HID_KEY_X, MOD_NONE}, {HID_KEY_Y, MOD_NONE},
    {HID_KEY_Z, MOD_NONE},

    // 123-127: 記号 ( {, |, }, ~, DEL )
    {HID_KEY_BRACKET_LEFT, MOD_SHIFT},      // '{' (123)
    {HID_KEY_BACKSLASH, MOD_SHIFT},         // '|'
    {HID_KEY_BRACKET_RIGHT, MOD_SHIFT},     // '}'
    {HID_KEY_GRAVE, MOD_SHIFT},             // '~'
    {HID_KEY_DELETE, MOD_NONE}              // DEL (127)
};

// JISキーマップ定義 (日本語キーボード設定のPCに接続する場合のマッピング)
const KeyMapping keymap_jis[128] = {
    // 0-31: 制御文字 (改行、タブなど)
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {HID_KEY_BACKSPACE, MOD_NONE}, {HID_KEY_TAB, MOD_NONE}, {HID_KEY_ENTER, MOD_NONE}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

    // 32-47: スペースおよび記号 ( , !, ", #, $, %, &, ', (, ), *, +, ,, -, ., / )
    {HID_KEY_SPACE, MOD_NONE},              // ' ' (32)
    {HID_KEY_1, MOD_SHIFT},                 // '!'
    {HID_KEY_2, MOD_SHIFT},                 // '"'  -> JIS: Shift + 2
    {HID_KEY_3, MOD_SHIFT},                 // '#'  -> JIS: Shift + 3
    {HID_KEY_4, MOD_SHIFT},                 // '$'  -> JIS: Shift + 4
    {HID_KEY_5, MOD_SHIFT},                 // '%'  -> JIS: Shift + 5
    {HID_KEY_6, MOD_SHIFT},                 // '&'  -> JIS: Shift + 6
    {HID_KEY_7, MOD_SHIFT},                 // '\'' -> JIS: Shift + 7
    {HID_KEY_8, MOD_SHIFT},                 // '('  -> JIS: Shift + 8
    {HID_KEY_9, MOD_SHIFT},                 // ')'  -> JIS: Shift + 9
    {HID_KEY_APOSTROPHE, MOD_SHIFT},        // '*'  -> JIS: Shift + :
    {HID_KEY_SEMICOLON, MOD_SHIFT},         // '+'  -> JIS: Shift + ;
    {HID_KEY_COMMA, MOD_NONE},              // ','
    {HID_KEY_MINUS, MOD_NONE},              // '-'
    {HID_KEY_PERIOD, MOD_NONE},             // '.'
    {HID_KEY_SLASH, MOD_NONE},              // '/' (47)

    // 48-57: 数字 (0-9)
    {HID_KEY_0, MOD_NONE}, {HID_KEY_1, MOD_NONE}, {HID_KEY_2, MOD_NONE}, {HID_KEY_3, MOD_NONE}, {HID_KEY_4, MOD_NONE},
    {HID_KEY_5, MOD_NONE}, {HID_KEY_6, MOD_NONE}, {HID_KEY_7, MOD_NONE}, {HID_KEY_8, MOD_NONE}, {HID_KEY_9, MOD_NONE},

    // 58-64: 記号 ( :, ;, <, =, >, ?, @ )
    {HID_KEY_APOSTROPHE, MOD_NONE},         // ':' -> JIS: : (単体)
    {HID_KEY_SEMICOLON, MOD_NONE},          // ';' -> JIS: ; (単体)
    {HID_KEY_COMMA, MOD_SHIFT},             // '<' -> JIS: Shift + ,
    {HID_KEY_MINUS, MOD_SHIFT},             // '=' -> JIS: Shift + -
    {HID_KEY_PERIOD, MOD_SHIFT},            // '>' -> JIS: Shift + .
    {HID_KEY_SLASH, MOD_SHIFT},             // '?' -> JIS: Shift + /
    {HID_KEY_BRACKET_LEFT, MOD_NONE},       // '@' -> JIS: @ (単体) (64)

    // 65-90: 大文字 (A-Z)
    {HID_KEY_A, MOD_SHIFT}, {HID_KEY_B, MOD_SHIFT}, {HID_KEY_C, MOD_SHIFT}, {HID_KEY_D, MOD_SHIFT}, {HID_KEY_E, MOD_SHIFT},
    {HID_KEY_F, MOD_SHIFT}, {HID_KEY_G, MOD_SHIFT}, {HID_KEY_H, MOD_SHIFT}, {HID_KEY_I, MOD_SHIFT}, {HID_KEY_J, MOD_SHIFT},
    {HID_KEY_K, MOD_SHIFT}, {HID_KEY_L, MOD_SHIFT}, {HID_KEY_M, MOD_SHIFT}, {HID_KEY_N, MOD_SHIFT}, {HID_KEY_O, MOD_SHIFT},
    {HID_KEY_P, MOD_SHIFT}, {HID_KEY_Q, MOD_SHIFT}, {HID_KEY_R, MOD_SHIFT}, {HID_KEY_S, MOD_SHIFT}, {HID_KEY_T, MOD_SHIFT},
    {HID_KEY_U, MOD_SHIFT}, {HID_KEY_V, MOD_SHIFT}, {HID_KEY_W, MOD_SHIFT}, {HID_KEY_X, MOD_SHIFT}, {HID_KEY_Y, MOD_SHIFT},
    {HID_KEY_Z, MOD_SHIFT},

    // 91-96: 記号 ( [, \, ], ^, _, ` )
    {HID_KEY_BRACKET_RIGHT, MOD_NONE},      // '['  -> JIS: [ (単体)
    {HID_KEY_KANJI3, MOD_NONE},             // '\'  -> JIS: ￥ (単体)
    {HID_KEY_BACKSLASH, MOD_NONE},          // ']'  -> JIS: ] (単体)
    {HID_KEY_EQUAL, MOD_NONE},              // '^'  -> JIS: ^ (単体)
    {HID_KEY_KANJI1, MOD_SHIFT},            // '_'  -> JIS: Shift + ろ
    {HID_KEY_BRACKET_LEFT, MOD_SHIFT},      // '`'  -> JIS: Shift + @ (96)

    // 97-122: 小文字 (a-z)
    {HID_KEY_A, MOD_NONE}, {HID_KEY_B, MOD_NONE}, {HID_KEY_C, MOD_NONE}, {HID_KEY_D, MOD_NONE}, {HID_KEY_E, MOD_NONE},
    {HID_KEY_F, MOD_NONE}, {HID_KEY_G, MOD_NONE}, {HID_KEY_H, MOD_NONE}, {HID_KEY_I, MOD_NONE}, {HID_KEY_J, MOD_NONE},
    {HID_KEY_K, MOD_NONE}, {HID_KEY_L, MOD_NONE}, {HID_KEY_M, MOD_NONE}, {HID_KEY_N, MOD_NONE}, {HID_KEY_O, MOD_NONE},
    {HID_KEY_P, MOD_NONE}, {HID_KEY_Q, MOD_NONE}, {HID_KEY_R, MOD_NONE}, {HID_KEY_S, MOD_NONE}, {HID_KEY_T, MOD_NONE},
    {HID_KEY_U, MOD_NONE}, {HID_KEY_V, MOD_NONE}, {HID_KEY_W, MOD_NONE}, {HID_KEY_X, MOD_NONE}, {HID_KEY_Y, MOD_NONE},
    {HID_KEY_Z, MOD_NONE},

    // 123-127: 記号 ( {, |, }, ~, DEL )
    {HID_KEY_BRACKET_RIGHT, MOD_SHIFT},     // '{'  -> JIS: Shift + [ (123)
    {HID_KEY_KANJI3, MOD_SHIFT},            // '|'  -> JIS: Shift + ￥
    {HID_KEY_BACKSLASH, MOD_SHIFT},         // '}'  -> JIS: Shift + ]
    {HID_KEY_EQUAL, MOD_SHIFT},             // '~'  -> JIS: Shift + ^
    {HID_KEY_DELETE, MOD_NONE}              // DEL (127)
};
