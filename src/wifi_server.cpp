#include "wifi_server.h"
#include "usb_hid.h"
#include "wifi_credentials.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "cJSON.h"

static const char* TAG = "WIFI_SERVER";
#define MAX_TEXT_LEN 2048

// 受信データバッファと排他制御
static char s_received_text[MAX_TEXT_LEN] = "";
static bool s_received_is_jis = true;
static bool s_received_direct = false;
static bool s_has_new_text = false;
static SemaphoreHandle_t s_data_mutex = NULL;

// Wi-Fi 接続状態とIPアドレス
static bool s_wifi_connected = false;
static char s_ip_addr[32] = "0.0.0.0";
static SemaphoreHandle_t s_status_mutex = NULL;

// HTTP サーバーのインスタンス
static httpd_handle_t s_server = NULL;

// ------------------------------------------------------------------
// 近未来的警告色調 Web UI HTMLデータ (即時送信ボタン追加)
// ------------------------------------------------------------------
static const char* s_index_html = R"rawhtml(
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>KEYSTREAM TRANSMITTER</title>
    <style>
        html, body {
            height: 100%;
            margin: 0;
            background-color: #000;
        }
        body {
            color: #ff5a00;
            font-family: 'Courier New', Courier, monospace;
            padding: 20px;
            box-sizing: border-box;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container {
            width: 100%;
            max-width: 500px;
            border: 2px solid #ff5a00;
            padding: 25px;
            background-color: #120500;
            box-shadow: 0 0 20px rgba(255, 90, 0, 0.4);
            box-sizing: border-box;
        }
        h1 {
            text-align: center;
            font-size: 20px;
            border-bottom: 2px solid #ff5a00;
            padding-bottom: 10px;
            margin-top: 0;
            letter-spacing: 2px;
        }
        label {
            display: block;
            margin: 15px 0 5px;
            font-size: 14px;
        }
        textarea {
            width: 100%;
            height: 180px;
            background-color: #000;
            border: 1px solid #ff5a00;
            color: #fff;
            padding: 12px;
            box-sizing: border-box;
            font-size: 16px;
            resize: none;
        }
        textarea:focus {
            outline: none;
            box-shadow: 0 0 5px #ff5a00;
        }
        .radio-group {
            display: flex;
            gap: 20px;
            margin: 15px 0;
        }
        .radio-label {
            display: flex;
            align-items: center;
            gap: 5px;
            cursor: pointer;
        }
        input[type="radio"] {
            accent-color: #ff5a00;
        }
        .button-group {
            display: flex;
            gap: 15px;
            margin-top: 15px;
        }
        button {
            flex: 1;
            padding: 12px;
            border: none;
            font-weight: bold;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.2s;
            letter-spacing: 1px;
        }
        .btn-sub {
            background-color: #ff5a00;
            color: #000;
        }
        .btn-sub:hover {
            background-color: #ff7830;
            box-shadow: 0 0 10px rgba(255, 90, 0, 0.5);
        }
        .btn-main {
            background-color: #ff0000;
            color: #fff;
            border: 1px solid #ff5a00;
        }
        .btn-main:hover {
            background-color: #ff3333;
            box-shadow: 0 0 10px rgba(255, 0, 0, 0.5);
        }
        .status-panel {
            margin-top: 20px;
            border: 1px solid #320a0a;
            background-color: #0a0202;
            padding: 10px;
            font-size: 12px;
        }
        .status-item {
            display: flex;
            justify-content: space-between;
            margin: 5px 0;
        }
        .status-val {
            font-weight: bold;
        }
        .online { color: #00ff64; }
        .offline { color: #ff0000; }
    </style>
</head>
<body>
    <div class="container">
        <h1>KEYSTREAM TRANSMITTER</h1>
        <form id="txForm">
            <label for="text">TRANSMIT BUFFER DATA:</label>
            <textarea id="text" placeholder="送信する文字列を入力してください..."></textarea>
            
            <label>KEYBOARD LAYOUT:</label>
            <div class="radio-group">
                <label class="radio-label">
                    <input type="radio" name="layout" value="jis" checked> JIS (日本語)
                </label>
                <label class="radio-label">
                    <input type="radio" name="layout" value="us"> US (英語)
                </label>
            </div>
            
            <div class="button-group">
                <button type="button" id="btnBuffer" class="btn-sub">BUFFER ONLY</button>
                <button type="button" id="btnDirect" class="btn-main">DIRECT SEND</button>
            </div>
        </form>

        <div class="status-panel">
            <div class="status-item">
                <span>SYSTEM STATUS:</span>
                <span id="sysStatus" class="online">ACTIVE</span>
            </div>
            <div class="status-item">
                <span>USB KEYBOARD:</span>
                <span id="usbStatus" class="offline">UNKNOWN</span>
            </div>
            <div class="status-item">
                <span>TRANSMITTING:</span>
                <span id="txStatus">IDLE</span>
            </div>
        </div>
    </div>

    <script>
        const btnBuffer = document.getElementById('btnBuffer');
        const btnDirect = document.getElementById('btnDirect');
        const usbStatus = document.getElementById('usbStatus');
        const txStatus = document.getElementById('txStatus');

        async function updateStatus() {
            try {
                const res = await fetch('/api/status');
                const data = await res.json();
                
                if (data.usb_mounted) {
                    usbStatus.textContent = 'MOUNTED';
                    usbStatus.className = 'status-val online';
                } else {
                    usbStatus.textContent = 'NOT MOUNTED';
                    usbStatus.className = 'status-val offline';
                }

                if (data.usb_sending) {
                    txStatus.textContent = 'TYPING...';
                    txStatus.style.color = '#ffcc00';
                } else {
                    txStatus.textContent = 'IDLE';
                    txStatus.style.color = '#ff5a00';
                }
                
                document.getElementById('sysStatus').textContent = 'ACTIVE';
                document.getElementById('sysStatus').className = 'status-val online';
            } catch (e) {
                document.getElementById('sysStatus').textContent = 'OFFLINE';
                document.getElementById('sysStatus').className = 'status-val offline';
            }
        }

        setInterval(updateStatus, 2000);
        updateStatus();

        async function sendData(direct) {
            const text = document.getElementById('text').value;
            const layout = document.querySelector('input[name="layout"]:checked').value;
            
            if (!text) return;

            try {
                const res = await fetch('/api/send', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ text, layout, direct })
                });
                const data = await res.json();
                if (data.status === 'ok') {
                    if (direct) {
                        alert('データをPCへ即時送信しました。');
                    } else {
                        alert('データをM5Stackの送信バッファに転送しました。\n本体の画面をタップしてPCへ送信してください。');
                    }
                    document.getElementById('text').value = '';
                } else {
                    alert('送信エラーが発生しました。');
                }
            } catch (err) {
                alert('通信に失敗しました。');
            }
        }

        btnBuffer.addEventListener('click', () => sendData(false));
        btnDirect.addEventListener('click', () => sendData(true));
    </script>
</body>
</html>
)rawhtml";

// ------------------------------------------------------------------
// Wi-Fi / IP イベントハンドラ
// ------------------------------------------------------------------
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Wi-Fi connecting started...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            s_wifi_connected = false;
            strcpy(s_ip_addr, "0.0.0.0");
            xSemaphoreGive(s_status_mutex);
        }
        ESP_LOGI(TAG, "Wi-Fi disconnected. Reconnecting...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            esp_ip4addr_ntoa(&event->ip_info.ip, s_ip_addr, sizeof(s_ip_addr));
            s_wifi_connected = true;
            xSemaphoreGive(s_status_mutex);
        }
        ESP_LOGI(TAG, "Wi-Fi connected. Got IP: %s", s_ip_addr);
    }
}

// ------------------------------------------------------------------
// APIハンドラの実装
// ------------------------------------------------------------------

// GET / (Web UIの配信)
static esp_err_t index_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, s_index_html, strlen(s_index_html));
    return ESP_OK;
}

// POST /api/send
static esp_err_t api_send_post_handler(httpd_req_t *req) {
    char buf[1024];
    int ret, remaining = req->content_len;
    
    if (req->content_len >= MAX_TEXT_LEN) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }

    char* content = (char*)malloc(req->content_len + 1);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }
    
    char* p = content;
    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, sizeof(buf) < remaining ? sizeof(buf) : remaining)) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            free(content);
            return ESP_FAIL;
        }
        memcpy(p, buf, ret);
        p += ret;
        remaining -= ret;
    }
    *p = '\0';

    ESP_LOGI(TAG, "Received JSON: %s", content);

    cJSON *json = cJSON_Parse(content);
    free(content);
    
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *text_item = cJSON_GetObjectItemCaseSensitive(json, "text");
    cJSON *layout_item = cJSON_GetObjectItemCaseSensitive(json, "layout");
    cJSON *direct_item = cJSON_GetObjectItemCaseSensitive(json, "direct");

    if (!cJSON_IsString(text_item) || (text_item->valuestring == NULL)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'text' parameter");
        return ESP_FAIL;
    }

    bool is_jis = true;
    if (cJSON_IsString(layout_item) && layout_item->valuestring != NULL) {
        if (strcmp(layout_item->valuestring, "us") == 0) {
            is_jis = false;
        }
    }

    bool direct = false;
    if (cJSON_IsBool(direct_item)) {
        direct = cJSON_IsTrue(direct_item);
    }

    if (xSemaphoreTake(s_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        strncpy(s_received_text, text_item->valuestring, MAX_TEXT_LEN - 1);
        s_received_text[MAX_TEXT_LEN - 1] = '\0';
        s_received_is_jis = is_jis;
        s_received_direct = direct;
        s_has_new_text = true;
        xSemaphoreGive(s_data_mutex);
        ESP_LOGI(TAG, "Buffered new text. Layout: %s, Direct: %s", is_jis ? "JIS" : "US", direct ? "true" : "false");
    } else {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Buffer mutex timeout");
        return ESP_FAIL;
    }

    cJSON_Delete(json);

    char resp_str[128];
    snprintf(resp_str, sizeof(resp_str), "{\"status\":\"ok\",\"length\":%d}", strlen(s_received_text));
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

// GET /api/status
static esp_err_t api_status_get_handler(httpd_req_t *req) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "usb_mounted", usb_hid_is_mounted());
    cJSON_AddBoolToObject(root, "usb_sending", usb_hid_is_sending());
    
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        cJSON_AddBoolToObject(root, "wifi_connected", s_wifi_connected);
        cJSON_AddStringToObject(root, "ip_address", s_ip_addr);
        xSemaphoreGive(s_status_mutex);
    }
    
    if (xSemaphoreTake(s_data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        cJSON_AddBoolToObject(root, "has_text", s_has_new_text);
        cJSON_AddNumberToObject(root, "text_length", strlen(s_received_text));
        cJSON_AddStringToObject(root, "layout", s_received_is_jis ? "jis" : "us");
        cJSON_AddBoolToObject(root, "direct", s_received_direct);
        xSemaphoreGive(s_data_mutex);
    }

    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, rendered, strlen(rendered));
    free(rendered);
    return ESP_OK;
}

// ------------------------------------------------------------------
// Wi-Fi & HTTP サーバー初期化 (STAモード)
// ------------------------------------------------------------------
void wifi_server_init(void) {
    ESP_LOGI(TAG, "Initializing Wi-Fi Client & HTTP Server");

    s_data_mutex = xSemaphoreCreateMutex();
    s_status_mutex = xSemaphoreCreateMutex();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASS);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi Client started. Target SSID: %s", WIFI_SSID);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&s_server, &config) == ESP_OK) {
        httpd_uri_t uri_index = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = index_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(s_server, &uri_index);

        httpd_uri_t uri_send = {
            .uri      = "/api/send",
            .method   = HTTP_POST,
            .handler  = api_send_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(s_server, &uri_send);

        httpd_uri_t uri_status = {
            .uri      = "/api/status",
            .method   = HTTP_GET,
            .handler  = api_status_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(s_server, &uri_status);
        
        ESP_LOGI(TAG, "HTTP Server started on port %d", config.server_port);
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP Server");
    }
}

bool wifi_server_get_text(char* buf, size_t max_len, bool* out_is_jis, bool* out_direct) {
    bool has_text = false;
    if (xSemaphoreTake(s_data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (s_has_new_text) {
            strncpy(buf, s_received_text, max_len - 1);
            buf[max_len - 1] = '\0';
            *out_is_jis = s_received_is_jis;
            *out_direct = s_received_direct;
            has_text = true;
        }
        xSemaphoreGive(s_data_mutex);
    }
    return has_text;
}

void wifi_server_clear_text(void) {
    if (xSemaphoreTake(s_data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        s_has_new_text = false;
        s_received_text[0] = '\0';
        s_received_direct = false;
        xSemaphoreGive(s_data_mutex);
    }
}

bool wifi_server_has_new_text(void) {
    bool has_text = false;
    if (xSemaphoreTake(s_data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        has_text = s_has_new_text;
        xSemaphoreGive(s_data_mutex);
    }
    return has_text;
}

void wifi_server_get_ip(char* buf, size_t max_len) {
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        strncpy(buf, s_ip_addr, max_len - 1);
        buf[max_len - 1] = '\0';
        xSemaphoreGive(s_status_mutex);
    }
}

bool wifi_server_is_connected(void) {
    bool connected = false;
    if (xSemaphoreTake(s_status_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        connected = s_wifi_connected;
        xSemaphoreGive(s_status_mutex);
    }
    return connected;
}
