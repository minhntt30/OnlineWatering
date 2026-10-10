#include "watchdog.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_log.h"
#include "soc/soc_caps.h"

static const char *TAG = "WATCHDOG";

void watchdog_init(void)
{
    // Tell us if the previous boot ended because of a freeze or crash
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT ||
        reason == ESP_RST_WDT || reason == ESP_RST_PANIC) {
        ESP_LOGW(TAG, "Last reset was caused by a watchdog or panic (reason %d)", (int)reason);
    }

    esp_task_wdt_config_t cfg = {
        .timeout_ms     = WATCHDOG_TIMEOUT_MS,
        .idle_core_mask = (1 << SOC_CPU_CORES_NUM) - 1,   // watch the idle task on every core
        .trigger_panic  = true,                           // panic = reboot, not just a warning
    };

    // ESP-IDF normally starts the TWDT itself: reconfigure it, or start it if it was disabled
    esp_err_t err = esp_task_wdt_reconfigure(&cfg);
    if (err == ESP_ERR_INVALID_STATE) {
        err = esp_task_wdt_init(&cfg);
    }
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "Watchdog armed: %d ms", WATCHDOG_TIMEOUT_MS);
}

void watchdog_add_task(void)
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
}

void watchdog_feed(void)
{
    esp_task_wdt_reset();
}