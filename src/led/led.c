#include "led.h"
#include <stddef.h>
#include "driver/gpio.h"
#include "esp_timer.h"

// --- Onboard LED (most ESP32 DevKit boards: GPIO2, active HIGH) ---
#define LED_GPIO      GPIO_NUM_2
#define LED_ON_LEVEL  1
#define LED_STEP_MS   100

// One character per LED_STEP_MS: '1' = LED on, '0' = LED off, repeated forever
static const char *s_patterns[] = {
    [LED_STATE_WIFI_CONNECTING] = "1100",
    [LED_STATE_MQTT_CONNECTING] = "1010000000",
    [LED_STATE_ONLINE]          = "10000000000000000000",
    [LED_STATE_ERROR]           = "10",
};

static volatile led_state_t s_state = LED_STATE_WIFI_CONNECTING;
static volatile bool s_pump_active = false;

static esp_timer_handle_t s_led_timer = NULL;
static led_state_t s_last_state = LED_STATE_WIFI_CONNECTING;
static size_t s_step = 0;

/* Runs every LED_STEP_MS in the shared esp_timer task: keep it short, never block */
static void led_timer_cb(void *arg)
{
    led_state_t state = s_state;
    if (state != s_last_state) {
        s_last_state = state;
        s_step = 0;
    }

    bool on = true;   // pump running: solid ON
    if (!s_pump_active) {
        const char *pattern = s_patterns[state];
        if (pattern[s_step] == '\0') {
            s_step = 0;
        }
        on = (pattern[s_step] == '1');
        s_step++;
    }

    gpio_set_level(LED_GPIO, on ? LED_ON_LEVEL : !LED_ON_LEVEL);
}

void led_init(void)
{
    // LED off before the pin becomes an output
    gpio_set_level(LED_GPIO, !LED_ON_LEVEL);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, !LED_ON_LEVEL);

    s_last_state = s_state;
    s_step = 0;

    const esp_timer_create_args_t timer_args = {
        .callback = led_timer_cb,
        .name     = "led",
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &s_led_timer));

    led_timer_cb(NULL);   // show the first step immediately
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_led_timer, (uint64_t)LED_STEP_MS * 1000));
}

void led_set_state(led_state_t state)
{
    s_state = state;
}

void led_set_pump_active(bool active)
{
    s_pump_active = active;
}