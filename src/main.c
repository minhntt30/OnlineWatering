#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi/wifi.h"
#include "mqtt/mqtt.h"
#include "pump/pump.h"
#include "watchdog/watchdog.h"
#include "led/led.h"


static const char *TAG = "MAIN";

void app_main(void)
{
    // Initialize NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing Pump...");
    pump_init();

    ESP_LOGI(TAG, "Initializing Watchdog...");
    watchdog_init();

    ESP_LOGI(TAG, "Initializing LED...");
    led_init();

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    if (wifi_init_sta() == ESP_OK) {
        led_set_state(LED_STATE_MQTT_CONNECTING);
    }

    ESP_LOGI(TAG, "Starting MQTT Client...");
    mqtt_app_start();
}