#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Wi-Fi STAモード & HTTP Serverの初期化
void wifi_server_init(void);

// 受信テキストを取得する (排他制御)
// out_direct に即時送信フラグが格納される
bool wifi_server_get_text(char* buf, size_t max_len, bool* out_is_jis, bool* out_direct);

// 受信テキストのクリア
void wifi_server_clear_text(void);

// 新規テキストがあるかどうか
bool wifi_server_has_new_text(void);

// IPアドレスの取得
void wifi_server_get_ip(char* buf, size_t max_len);

// Wi-Fi接続状態の取得
bool wifi_server_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_SERVER_H
