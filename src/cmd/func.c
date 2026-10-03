#include "func.h"
#include "esp_log.h"

static const char *TAG = "CMD_FUNC";

/* --- Command functions: add or remove here --- */

void led_control(const char *data, int data_len)
{
    ESP_LOGI(TAG, "LED control: %.*s", data_len, data);
}
