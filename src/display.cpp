#include "display.h"
#include <M5Unified.h>
#include <string.h>

// 近未来的警告色調（ダークレッド/オレンジ/グリーン）のカラー定義 (RGB565)
#define COLOR_UI_ORANGE      M5.Display.color565(255, 90, 0)     // メインオレンジ
#define COLOR_UI_AMBER       M5.Display.color565(230, 70, 0)     // やや暗い琥珀色
#define COLOR_UI_DARK_RED    M5.Display.color565(50, 10, 10)     // 背景用暗赤色
#define COLOR_UI_DARK_ORANGE M5.Display.color565(35, 12, 0)      // ボタン背景用暗橙色
#define COLOR_UI_GREEN       M5.Display.color565(0, 255, 100)    // 蛍光グリーン (安全・接続状態)
#define COLOR_UI_GRAY        M5.Display.color565(60, 60, 60)     // 無効時のグレー
#define COLOR_UI_RED         M5.Display.color565(255, 0, 0)      // 警告赤

// Wi-Fiアイコン自前描画 (アンテナバー4本)
static void draw_wifi_icon(int x, int y, bool connected) {
    uint16_t color = connected ? COLOR_UI_GREEN : COLOR_UI_GRAY;
    for (int i = 0; i < 4; i++) {
        int height = (i + 1) * 4;
        int bar_y = y - height + 12;
        M5.Display.fillRect(x + (i * 4), bar_y, 2, height, color);
    }
}

// USBアイコン自前描画 (USBコネクタ形状)
static void draw_usb_icon(int x, int y, bool mounted) {
    uint16_t color = mounted ? COLOR_UI_GREEN : COLOR_UI_GRAY;
    // コネクタ本体
    M5.Display.drawRect(x, y + 2, 10, 8, color);
    M5.Display.fillRect(x + 2, y + 4, 2, 2, color);
    M5.Display.fillRect(x + 6, y + 4, 2, 2, color);
    // 金属端子部
    M5.Display.fillRect(x + 10, y + 3, 4, 6, color);
    // 接続線
    M5.Display.drawLine(x - 4, y + 6, x, y + 6, color);
}

void display_init(void) {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1); // 横向き
    M5.Display.clear(TFT_BLACK);
    
    M5.Display.startWrite();
    M5.Display.fillRect(0, 0, 320, 240, TFT_BLACK);
    M5.Display.drawRect(5, 5, 310, 230, COLOR_UI_ORANGE);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(COLOR_UI_ORANGE, TFT_BLACK);
    M5.Display.drawCenterString("KEYSTREAM SYSTEM", 160, 60);
    M5.Display.setTextSize(1.5);
    M5.Display.drawCenterString("BOOTING SYSTEM...", 160, 120);
    M5.Display.endWrite();
}

void display_update(bool wifi_connected, const char* ip_addr, bool usb_mounted, bool usb_sending, const char* rx_text, const struct HistoryItem* history, int history_count) {
    M5.Display.startWrite();
    
    // 1. ヘッダーバー (Y: 0〜30)
    M5.Display.fillRect(0, 0, 320, 30, COLOR_UI_DARK_RED);
    M5.Display.drawLine(0, 30, 320, 30, COLOR_UI_ORANGE);
    
    // Wi-Fi アイコンとIPアドレス (左側)
    draw_wifi_icon(8, 8, wifi_connected);
    M5.Display.setTextColor(wifi_connected ? COLOR_UI_GREEN : COLOR_UI_GRAY, COLOR_UI_DARK_RED);
    M5.Display.setTextSize(1);
    if (wifi_connected) {
        M5.Display.drawString(ip_addr, 30, 10);
    } else {
        M5.Display.drawString("OFFLINE", 30, 10);
    }
    
    // システムタイトル (中央X:135に固定してIPアドレスとの被りを回避)
    M5.Display.setTextColor(COLOR_UI_ORANGE, COLOR_UI_DARK_RED);
    M5.Display.setTextSize(1.2);
    M5.Display.drawString("KEYSTREAM", 135, 8);
    
    // USB アイコン (右側)
    draw_usb_icon(295, 8, usb_mounted);
    
    // 2. メインエリア背景クリア
    M5.Display.fillRect(0, 31, 320, 209, TFT_BLACK);
    
    // 3. 最新バッファ（未確認データ）エリア (Y: 35〜95)
    uint16_t buffer_border_color = usb_sending ? COLOR_UI_RED : COLOR_UI_ORANGE;
    uint16_t buffer_bg = COLOR_UI_DARK_ORANGE;
    
    M5.Display.fillRect(5, 35, 310, 60, buffer_bg);
    M5.Display.drawRect(5, 35, 310, 60, buffer_border_color);
    
    M5.Display.setTextColor(buffer_border_color, buffer_bg);
    M5.Display.setTextSize(1);
    if (usb_sending) {
        M5.Display.drawString(">>> TRANSMITTING DATA <<<", 12, 40);
    } else {
        M5.Display.drawString("UNCONFIRMED DATA (TAP TO SEND)", 12, 40);
    }
    
    M5.Display.setTextColor(TFT_WHITE, buffer_bg);
    M5.Display.setTextSize(1.2);
    if (rx_text && strlen(rx_text) > 0) {
        char trunc[32];
        strncpy(trunc, rx_text, 28);
        trunc[28] = '\0';
        if (strlen(rx_text) > 28) {
            strcat(trunc, "...");
        }
        M5.Display.drawString(trunc, 15, 65);
    } else {
        M5.Display.setTextColor(COLOR_UI_GRAY, buffer_bg);
        M5.Display.drawString("NO NEW DATA", 15, 65);
    }
    
    // 4. 送信履歴エリア (Y: 100〜235)
    M5.Display.setTextColor(COLOR_UI_ORANGE, TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.drawString("TRANSMISSION HISTORY", 10, 102);
    M5.Display.drawLine(10, 113, 310, 113, COLOR_UI_AMBER);
    
    // 履歴スロット描画
    for (int i = 0; i < 3; i++) {
        int slot_y = 118 + (i * 40);
        
        if (i < history_count && history[i].active) {
            M5.Display.fillRect(5, slot_y, 310, 35, COLOR_UI_DARK_ORANGE);
            M5.Display.drawRect(5, slot_y, 310, 35, COLOR_UI_AMBER);
            
            // [A] インデックス表示 (例: [1])
            M5.Display.setTextColor(COLOR_UI_ORANGE, COLOR_UI_DARK_ORANGE);
            M5.Display.setTextSize(1);
            char idx_str[8];
            snprintf(idx_str, sizeof(idx_str), "[%d]", i + 1);
            M5.Display.drawString(idx_str, 10, slot_y + 10);
            
            // [B] JIS/US ラベルのミリタリー調リバース描画 (オレンジ背景に黒文字)
            M5.Display.fillRect(32, slot_y + 8, 28, 18, COLOR_UI_AMBER);
            M5.Display.setTextColor(TFT_BLACK, COLOR_UI_AMBER);
            M5.Display.drawString(history[i].is_jis ? "JIS" : "US", 36, slot_y + 10);
            
            // [C] 右端パーテーション線と文字数表示
            M5.Display.drawLine(260, slot_y + 2, 260, slot_y + 32, COLOR_UI_AMBER);
            M5.Display.setTextColor(COLOR_UI_AMBER, COLOR_UI_DARK_ORANGE);
            M5.Display.drawString("LEN", 265, slot_y + 10);
            
            char len_str[16];
            snprintf(len_str, sizeof(len_str), "%d", (int)strlen(history[i].text));
            M5.Display.setTextColor(TFT_WHITE, COLOR_UI_DARK_ORANGE);
            M5.Display.drawRightString(len_str, 304, slot_y + 10);
            
            // [D] 本文テキスト (パーテーションに被らないように切り詰めて表示)
            char hist_trunc[24];
            strncpy(hist_trunc, history[i].text, 20);
            hist_trunc[20] = '\0';
            if (strlen(history[i].text) > 20) {
                strcat(hist_trunc, "..");
            }
            M5.Display.drawString(hist_trunc, 68, slot_y + 10);
        } else {
            M5.Display.drawRect(5, slot_y, 310, 35, COLOR_UI_DARK_RED);
            M5.Display.setTextColor(COLOR_UI_DARK_RED, TFT_BLACK);
            M5.Display.setTextSize(1);
            char empty_str[16];
            snprintf(empty_str, sizeof(empty_str), "[%d] EMPTY", i + 1);
            M5.Display.drawString(empty_str, 12, slot_y + 10);
        }
    }
    
    M5.Display.endWrite();
}

int display_get_touched_area(int x, int y) {
    if (x < 5 || x > 315) return -1;
    
    // 最新バッファエリア (Y: 35〜95)
    if (y >= 35 && y <= 95) {
        return 0;
    }
    
    // 履歴スロット 1 (Y: 118〜153)
    if (y >= 118 && y <= 153) {
        return 1;
    }
    
    // 履歴スロット 2 (Y: 158〜193)
    if (y >= 158 && y <= 193) {
        return 2;
    }
    
    // 履歴スロット 3 (Y: 198〜233)
    if (y >= 198 && y <= 233) {
        return 3;
    }
    
    return -1;
}
