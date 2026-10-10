#pragma once
#include <stdbool.h>

typedef enum {
    LED_STATE_WIFI_CONNECTING,   // fast blink
    LED_STATE_MQTT_CONNECTING,   // double blink
    LED_STATE_ONLINE,            // short heartbeat flash every 2 s
    LED_STATE_ERROR,             // very fast blink (gave up reconnecting)
} led_state_t;

// Sets up the LED pin and starts the blink task. Initial state: LED_STATE_WIFI_CONNECTING.
void led_init(void);

// Select the connection status pattern (safe to call from any task)
void led_set_state(led_state_t state);

// While true the LED stays solid ON (pump running), then the status pattern resumes
void led_set_pump_active(bool active);