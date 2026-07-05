#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdint.h>
#include "class/hid/hid.h"

// キーボード修飾キーの定義（TinyUSBのものと合わせる）
#define MOD_NONE          0
#define MOD_SHIFT         KEYBOARD_MODIFIER_LEFTSHIFT
#define MOD_CTRL          KEYBOARD_MODIFIER_LEFTCTRL
#define MOD_ALT           KEYBOARD_MODIFIER_LEFTALT

struct KeyMapping {
    uint8_t keycode;
    uint8_t modifier;
};

// ASCII 0-127 をキーコードに変換するテーブルの定義
extern const KeyMapping keymap_us[128];
extern const KeyMapping keymap_jis[128];

#endif // KEYMAP_H
