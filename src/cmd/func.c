#include "func.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "pump/pump.h"

static const char *TAG = "CMD_FUNC";

void pump_start_cmd(const char *data, int data_len)
{
    char buf[32];
    unsigned int duty;
    unsigned int duration_ms;

    // 'data' is not null-terminated: copy it before parsing
    if (data_len <= 0 || data_len >= (int)sizeof(buf)) {
        ESP_LOGW(TAG, "startpump: bad payload length");
        return;
    }
    memcpy(buf, data, data_len);
    buf[data_len] = '\0';

    if (sscanf(buf, "%u,%u", &duty, &duration_ms) != 2 || duty > 100) {
        ESP_LOGW(TAG, "startpump: expected \"duty,duration_ms\", got \"%s\"", buf);
        return;
    }
    // A running pump needs an explicit duration (duty 0 just stops it)
    if (duty > 0 && duration_ms == 0) {
        ESP_LOGW(TAG, "startpump: duration_ms must be > 0");
        return;
    }

    pump_start((uint8_t)duty, duration_ms);
}