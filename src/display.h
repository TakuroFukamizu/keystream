#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 送信履歴の項目構造体
struct HistoryItem {
    char text[128];
    bool is_jis;
    bool active;
};

// ディスプレイの初期化
void display_init(void);

// 画面の更新
void display_update(bool wifi_connected, const char* ip_addr, bool usb_mounted, bool usb_sending, const char* rx_text, const struct HistoryItem* history, int history_count);

// 指定した座標がどのボタンエリア内にあるか判定する
// 戻り値: -1=なし, 0=最新バッファ, 1=履歴1, 2=履歴2, 3=履歴3
int display_get_touched_area(int x, int y);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H
