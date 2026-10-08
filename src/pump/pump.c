#include "pump.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_log.h"

// --- LR7843 gate pin (ESP32-WROOM-32) ---
// GPIO25: not a strapping pin, not used by flash. Add a 10k pull-down gate->GND.
#define PUMP_GPIO         GPIO_NUM_25
#define PUMP_LEDC_MODE    LEDC_LOW_SPEED_MODE
#define PUMP_LEDC_TIMER   LEDC_TIMER_0
#define PUMP_LEDC_CHANNEL LEDC_CHANNEL_0
#define PUMP_PWM_FREQ_HZ  20000
#define PUMP_PWM_RES      LEDC_TIMER_10_BIT
#define PUMP_PWM_MAX_DUTY ((1 << 10) - 1)

static const char *TAG = "PUMP";
static esp_timer_handle_t s_timeout_timer = NULL;
static volatile bool s_running = false;

/* Hard timeout reached: force the pump OFF */
static void pump_timeout_cb(void *arg)
{
    ESP_LOGW(TAG, "Run time limit reached, stopping pump");
    pump_stop();
}

void pump_init(void)
{
    // Drive the pin LOW before it becomes an output so the MOSFET stays OFF
    gpio_set_level(PUMP_GPIO, 0);
    gpio_set_direction(PUMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(PUMP_GPIO, 0);

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = PUMP_LEDC_MODE,
        .duty_resolution = PUMP_PWM_RES,
        .timer_num       = PUMP_LEDC_TIMER,
        .freq_hz         = PUMP_PWM_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    // Duty starts at 0 so the pump stays OFF
    ledc_channel_config_t channel_cfg = {
        .gpio_num   = PUMP_GPIO,
        .speed_mode = PUMP_LEDC_MODE,
        .channel    = PUMP_LEDC_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = PUMP_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_cfg));

    const esp_timer_create_args_t timer_args = {
        .callback = pump_timeout_cb,
        .name     = "pump_timeout",
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &s_timeout_timer));

    pump_stop();
    ESP_LOGI(TAG, "Pump ready on GPIO%d, %d Hz PWM", PUMP_GPIO, PUMP_PWM_FREQ_HZ);
}

void pump_start(uint8_t duty_percent, uint32_t duration_ms)
{
    if (s_timeout_timer == NULL) {
        ESP_LOGE(TAG, "pump_init() was not called");
        return;
    }
    if (duty_percent == 0) {
        pump_stop();
        return;
    }
    if (duty_percent > 100) {
        duty_percent = 100;
    }
    if (duration_ms == 0 || duration_ms > PUMP_MAX_RUN_MS) {
        duration_ms = PUMP_MAX_RUN_MS;
    }

    // Arm the timeout BEFORE turning the pump on
    esp_timer_stop(s_timeout_timer);
    ESP_ERROR_CHECK(esp_timer_start_once(s_timeout_timer, (uint64_t)duration_ms * 1000));

    uint32_t duty = (uint32_t)duty_percent * PUMP_PWM_MAX_DUTY / 100;
    ESP_ERROR_CHECK(ledc_set_duty(PUMP_LEDC_MODE, PUMP_LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(PUMP_LEDC_MODE, PUMP_LEDC_CHANNEL));
    s_running = true;
    ESP_LOGI(TAG, "Pump ON: %u%% for %lu ms", duty_percent, (unsigned long)duration_ms);
}

void pump_stop(void)
{
    if (s_timeout_timer != NULL) {
        esp_timer_stop(s_timeout_timer);
    }
    // Hold the pin at idle level LOW
    ledc_stop(PUMP_LEDC_MODE, PUMP_LEDC_CHANNEL, 0);
    if (s_running) {
        ESP_LOGI(TAG, "Pump OFF");
    }
    s_running = false;
}

bool pump_is_running(void)
{
    return s_running;
}