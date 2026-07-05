#ifndef USB_HID_H
#define USB_HID_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// USB HIDの初期化
void usb_hid_init(void);

// 文字列の送信（JIS/US配列指定）
// 送信中は true を返し、送信中以外は false
bool usb_hid_send_string(const char* text, bool is_jis);

// 現在送信中かどうかを取得
bool usb_hid_is_sending(void);

// USBの接続状態（列挙状態）を取得
bool usb_hid_is_mounted(void);

#ifdef __cplusplus
}
#endif

#endif // USB_HID_H
