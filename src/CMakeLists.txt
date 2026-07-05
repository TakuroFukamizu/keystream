idf_component_register(SRCS "main.cpp" "usb_hid.cpp" "keymap.cpp" "wifi_server.cpp" "display.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_wifi esp_http_server nvs_flash json m5unified)
