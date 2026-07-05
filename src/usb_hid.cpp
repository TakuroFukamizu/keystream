#include "usb_hid.h"
#include "keymap.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"

static const char* TAG = "USB_HID";

// 接続状態管理
static bool s_usb_mounted = false;
static bool s_is_sending = false;

// 送信キュー
struct SendRequest {
    char* text;
    bool is_jis;
};
static QueueHandle_t s_send_queue = NULL;

// ------------------------------------------------------------------
// TinyUSB デスクリプタ定義
// ------------------------------------------------------------------

// デバイスデスクリプタ（CDC/MSC等を含まない純粋なHID Keyboard）
static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, // USB 2.0
    .bDeviceClass       = 0x00,   // インターフェース依存
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x303A, // Espressif VID
    .idProduct          = 0x4002, // 汎用HIDデバイス用PID
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// HID レポートデスクリプタ (Keyboard用)
static const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

// 構成デスクリプタ
#define EPNUM_HID   0x81
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

static const uint8_t desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10)
};

// 文字列デスクリプタ
static char const* string_desc_arr [] = {
    (const char[]) { 0x09, 0x04 }, // 0: Supported language is English (0x0409)
    "KeyStream Project",           // 1: Manufacturer
    "KeyStream Keyboard",          // 2: Product
    "KS-123456",                   // 3: Serials
};

// ------------------------------------------------------------------
// TinyUSB コールバックの実装 (Cリンケージ)
// 重複を避けるため、tud_descriptor_* は esp_tinyusb 内の標準実装を使用し、
// tinyusb_driver_install() 経由でポインタを登録します。
// ------------------------------------------------------------------
extern "C" {

// HIDレポートデスクリプタコールバックのみ自前で提供
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return desc_hid_report;
}

// マウント・アンマウント検出
void tud_mount_cb(void) {
    s_usb_mounted = true;
    ESP_LOGI(TAG, "USB Mounted");
}

void tud_umount_cb(void) {
    s_usb_mounted = false;
    ESP_LOGI(TAG, "USB Unmounted");
}

// サスペンド・レジューム
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    s_usb_mounted = false;
    ESP_LOGI(TAG, "USB Suspended");
}

void tud_resume_cb(void) {
    s_usb_mounted = true;
    ESP_LOGI(TAG, "USB Resumed");
}

// GET_REPORT/SET_REPORTはダミー実装でOK
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}

} // extern "C"

// ------------------------------------------------------------------
// キー送信ヘルパー
// ------------------------------------------------------------------
static void send_key(uint8_t keycode, uint8_t modifier) {
    // TinyUSBが準備できるまで待機 (切断時はタイムアウト)
    int timeout = 100;
    while (!tud_hid_ready() && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(2));
        timeout--;
    }
    if (timeout <= 0) return;

    // Key Down 送信
    uint8_t key_report[6] = {keycode, 0, 0, 0, 0, 0};
    tud_hid_keyboard_report(0, modifier, key_report);
    vTaskDelay(pdMS_TO_TICKS(15)); // 15ms 押下維持

    // Key Up 送信
    timeout = 100;
    while (!tud_hid_ready() && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(2));
        timeout--;
    }
    if (timeout <= 0) return;

    uint8_t empty_report[6] = {0, 0, 0, 0, 0, 0};
    tud_hid_keyboard_report(0, 0, empty_report);
    vTaskDelay(pdMS_TO_TICKS(25)); // キー間のウェイト 25ms
}

// ------------------------------------------------------------------
// 非推移的・非同期送信タスク
// ------------------------------------------------------------------
static void usb_hid_send_task(void* pvParameters) {
    SendRequest req;
    while (1) {
        if (xQueueReceive(s_send_queue, &req, portMAX_DELAY) == pdTRUE) {
            if (req.text == NULL) continue;
            
            s_is_sending = true;
            ESP_LOGI(TAG, "Start typing string, len=%d", strlen(req.text));

            const KeyMapping* current_map = req.is_jis ? keymap_jis : keymap_us;
            size_t len = strlen(req.text);

            for (size_t i = 0; i < len; i++) {
                // USBがマウントされていなければ送信中断
                if (!s_usb_mounted) {
                    ESP_LOGW(TAG, "USB disconnected during typing. Aborting.");
                    break;
                }

                uint8_t c = (uint8_t)req.text[i];
                if (c < 128) {
                    KeyMapping mapping = current_map[c];
                    // マッピングが存在するもの、あるいは制御文字を処理
                    if (mapping.keycode != 0 || c == '\n' || c == '\t' || c == ' ') {
                        send_key(mapping.keycode, mapping.modifier);
                    }
                }
            }

            ESP_LOGI(TAG, "Finished typing");

            // 安全のため最後にキーをすべて解放する
            int final_timeout = 100;
            while (!tud_hid_ready() && final_timeout > 0) {
                vTaskDelay(pdMS_TO_TICKS(2));
                final_timeout--;
            }
            uint8_t final_empty[6] = {0, 0, 0, 0, 0, 0};
            tud_hid_keyboard_report(0, 0, final_empty);

            free(req.text);
            s_is_sending = false;
        }
    }
}

// ------------------------------------------------------------------
// 公開 API の実装
// ------------------------------------------------------------------
void usb_hid_init(void) {
    ESP_LOGI(TAG, "Initializing USB HID");

    // キュー作成
    s_send_queue = xQueueCreate(2, sizeof(SendRequest));

    // TinyUSB設定
    tinyusb_config_t tusb_cfg = {};
    tusb_cfg.device_descriptor = &desc_device;
    tusb_cfg.configuration_descriptor = desc_configuration;
    tusb_cfg.string_descriptor = string_desc_arr;
    tusb_cfg.string_descriptor_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]);
    tusb_cfg.external_phy = false;
    tusb_cfg.self_powered = false;
    tusb_cfg.vbus_monitor_io = -1;
    
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // 送信タスク起動
    xTaskCreate(usb_hid_send_task, "usb_hid_send", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "USB HID initialized successfully");
}

bool usb_hid_send_string(const char* text, bool is_jis) {
    if (s_is_sending) {
        ESP_LOGW(TAG, "Already sending text");
        return false;
    }
    if (!s_usb_mounted) {
        ESP_LOGW(TAG, "USB not mounted, cannot send");
        return false;
    }

    SendRequest req;
    req.text = strdup(text);
    req.is_jis = is_jis;

    if (xQueueSend(s_send_queue, &req, 0) != pdTRUE) {
        free(req.text);
        return false;
    }

    return true;
}

bool usb_hid_is_sending(void) {
    return s_is_sending;
}

bool usb_hid_is_mounted(void) {
    return s_usb_mounted;
}
