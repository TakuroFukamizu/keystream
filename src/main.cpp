#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "usb_hid.h"
#include "wifi_server.h"
#include "display.h"
#include <M5Unified.h>

// 送信履歴の管理
static struct HistoryItem s_history[3] = {
    {"", true, false},
    {"", true, false},
    {"", true, false}
};

static void add_to_history(const char* text, bool is_jis) {
    if (text == NULL || strlen(text) == 0) return;

    // 既存の履歴から同一のものを検索
    int found_idx = -1;
    for (int i = 0; i < 3; i++) {
        if (s_history[i].active && strcmp(s_history[i].text, text) == 0 && s_history[i].is_jis == is_jis) {
            found_idx = i;
            break;
        }
    }

    if (found_idx != -1) {
        // すでに存在する場合、その項目より手前のものを後ろにずらして上書きする
        for (int i = found_idx; i > 0; i--) {
            s_history[i] = s_history[i - 1];
        }
    } else {
        // 新規項目の場合、すべてを押し出す
        for (int i = 2; i > 0; i--) {
            s_history[i] = s_history[i - 1];
        }
    }

    // 先頭に新規登録または再移動
    strncpy(s_history[0].text, text, sizeof(s_history[0].text) - 1);
    s_history[0].text[sizeof(s_history[0].text) - 1] = '\0';
    s_history[0].is_jis = is_jis;
    s_history[0].active = true;
}

extern "C" void app_main(void)
{
    // ログレベル設定
    esp_log_level_set("*", ESP_LOG_INFO);

    // ディスプレイ初期化 (M5Unifiedの初期化を内包)
    display_init();

    // USB HID 初期化
    usb_hid_init();

    // Wi-Fi AP & HTTP Server 初期化
    wifi_server_init();

    printf("KeyStream initialized.\n");

    // 未送信データ用バッファ
    char unconfirmed_text[2048] = "";
    bool unconfirmed_is_jis = true;
    
    // 状態監視用変数 (前回の値と異なるときだけ再描画してチラつき防止)
    bool last_wifi = false;
    char last_ip[32] = "0.0.0.0";
    bool last_usb = false;
    bool last_sending = false;
    char last_preview_text[2048] = "";

    // 初回描画
    display_update(last_wifi, last_ip, last_usb, last_sending, unconfirmed_text, s_history, 3);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50)); // タッチ追従性を上げるため50ms周期
        
        // M5Unifiedの内部状態更新
        M5.update();
        
        // 各種状態取得
        bool current_wifi = wifi_server_is_connected();
        char current_ip[32];
        wifi_server_get_ip(current_ip, sizeof(current_ip));
        bool current_usb = usb_hid_is_mounted();
        bool current_sending = usb_hid_is_sending();
        
        // 1. Wi-Fi経由で新規テキストを受信したかをチェック
        if (wifi_server_has_new_text()) {
            char rx_temp[2048];
            bool rx_jis;
            bool rx_direct = false;
            if (wifi_server_get_text(rx_temp, sizeof(rx_temp), &rx_jis, &rx_direct)) {
                printf("Buffered new text over Wi-Fi, len=%d. Layout: %s, Direct: %s\n", 
                       (int)strlen(rx_temp), rx_jis ? "JIS" : "US", rx_direct ? "true" : "false");
                
                if (rx_direct && current_usb && !current_sending) {
                    // 即時送信を実行
                    display_update(current_wifi, current_ip, current_usb, true, rx_temp, s_history, 3);
                    usb_hid_send_string(rx_temp, rx_jis);
                    add_to_history(rx_temp, rx_jis);
                    unconfirmed_text[0] = '\0';
                } else {
                    // 通常の保留モード
                    strncpy(unconfirmed_text, rx_temp, sizeof(unconfirmed_text) - 1);
                    unconfirmed_text[sizeof(unconfirmed_text) - 1] = '\0';
                    unconfirmed_is_jis = rx_jis;
                }
                
                wifi_server_clear_text();
            }
        }

        // 2. タッチ入力の検知
        if (M5.Touch.getCount() > 0) {
            auto detail = M5.Touch.getDetail(0);
            // タップ判定 (押されて離された、または押された瞬間)
            if (detail.wasPressed()) {
                int area = display_get_touched_area(detail.x, detail.y);
                
                if (area == 0) {
                    // 最新バッファがタップされた -> USBキーボード送信を実行
                    if (strlen(unconfirmed_text) > 0 && current_usb && !current_sending) {
                        printf("User tapped buffer. Typing: %s\n", unconfirmed_text);
                        
                        // 送信中であることを画面表示に即反映
                        display_update(current_wifi, current_ip, current_usb, true, unconfirmed_text, s_history, 3);
                        
                        // USB HID送信 (ブロッキング実行)
                        usb_hid_send_string(unconfirmed_text, unconfirmed_is_jis);
                        
                        // 履歴へ追加
                        add_to_history(unconfirmed_text, unconfirmed_is_jis);
                        
                        // 送信完了したのでバッファクリア
                        unconfirmed_text[0] = '\0';
                    }
                } 
                else if (area >= 1 && area <= 3) {
                    // 履歴スロットがタップされた -> 再送を実行
                    int idx = area - 1;
                    if (s_history[idx].active && current_usb && !current_sending) {
                        printf("User tapped history %d. Re-typing: %s\n", area, s_history[idx].text);
                        
                        // 送信テキストを退避
                        char resend_text[128];
                        strcpy(resend_text, s_history[idx].text);
                        bool resend_jis = s_history[idx].is_jis;
                        
                        // 送信中であることを画面表示に即反映
                        display_update(current_wifi, current_ip, current_usb, true, resend_text, s_history, 3);
                        
                        // 再送信
                        usb_hid_send_string(resend_text, resend_jis);
                        
                        // 再送したものを最新の履歴へ繰り上げ
                        add_to_history(resend_text, resend_jis);
                    }
                }
            }
        }

        // 3. 状態変化があったら液晶画面を更新
        if (current_wifi != last_wifi ||
            strcmp(current_ip, last_ip) != 0 ||
            current_usb != last_usb ||
            current_sending != last_sending ||
            strcmp(unconfirmed_text, last_preview_text) != 0) {
            
            last_wifi = current_wifi;
            strcpy(last_ip, current_ip);
            last_usb = current_usb;
            last_sending = current_sending;
            strncpy(last_preview_text, unconfirmed_text, sizeof(last_preview_text) - 1);
            last_preview_text[sizeof(last_preview_text) - 1] = '\0';
            
            display_update(current_wifi, current_ip, current_usb, current_sending, unconfirmed_text, s_history, 3);
        }
    }
}
