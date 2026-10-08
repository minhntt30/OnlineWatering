#pragma once
#include <stdint.h>
#include <stdbool.h>

// Hard limit for one continuous run (protects against water overflow)
#define PUMP_MAX_RUN_MS 60000

// Call first thing in app_main: forces the pump OFF and sets up PWM
void pump_init(void);

// Run the pump at duty_percent (1-100) for duration_ms, then stop automatically.
// duration_ms of 0 or above PUMP_MAX_RUN_MS is limited to PUMP_MAX_RUN_MS.
// duty_percent of 0 stops the pump.
void pump_start(uint8_t duty_percent, uint32_t duration_ms);

// Stop the pump immediately
void pump_stop(void);

bool pump_is_running(void);