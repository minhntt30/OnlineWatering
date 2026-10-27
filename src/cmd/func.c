#include "func.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "pump/pump.h"
#include "moisture/moisture.h"
#include "mqtt/mqtt.h"

static const char *TAG = "CMD_FUNC";

#define MOISTURE_VOLTAGE_TOPIC  "moisture/voltage"

/* --- Command functions: add or remove here --- */

void led_control(const char *data, int data_len)
{
    ESP_LOGI(TAG, "LED control: %.*s", data_len, data);
}

void pump_start_cmd(const char *data, int data_len)
{
    char buf[32];
    unsigned int duty;
    unsigned int duration_s;

    // 'data' is not null-terminated: copy it before parsing
    if (data_len <= 0 || data_len >= (int)sizeof(buf)) {
        ESP_LOGW(TAG, "startpump: bad payload length");
        return;
    }
    memcpy(buf, data, data_len);
    buf[data_len] = '\0';

    if (sscanf(buf, "%u,%u", &duty, &duration_s) != 2 || duty > 100) {
        ESP_LOGW(TAG, "startpump: expected \"duty,duration_s\", got \"%s\"", buf);
        return;
    }
    // A running pump needs an explicit duration (duty 0 just stops it)
    if (duty > 0 && duration_s == 0) {
        ESP_LOGW(TAG, "startpump: duration_s must be > 0");
        return;
    }

    pump_start((uint8_t)duty, duration_s * 1000);
}

void moisture_read_cmd(const char *data, int data_len)
{
    // The payload is not used: any message on the topic triggers a reading
    (void)data;
    (void)data_len;

    // Reads the ADC (several samples averaged) and converts it to volts using the ADC calibration
    float volts = moisture_read_voltage();
    if (volts < 0.0f) {
        ESP_LOGW(TAG, "getmoisture: sensor read failed");
        mqtt_publish_text(MOISTURE_VOLTAGE_TOPIC, "error");
        return;
    }

    // Send millivolts as integers ("1.234"): does not depend on float support in snprintf
    char msg[16];
    int mv = (int)(volts * 1000.0f + 0.5f);
    snprintf(msg, sizeof(msg), "%d.%03d", mv / 1000, mv % 1000);

    ESP_LOGI(TAG, "Moisture sensor: %s V", msg);
    if (!mqtt_publish_text(MOISTURE_VOLTAGE_TOPIC, msg)) {
        ESP_LOGW(TAG, "getmoisture: could not publish the result");
    }
}