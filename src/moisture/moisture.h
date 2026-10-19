#pragma once
#include "esp_err.h"

// Sets up the ADC channel for the resistive soil moisture sensor (call once at boot).
esp_err_t moisture_init(void);

// Averaged raw ADC reading (0..4095 at 12 bit). Returns -1 on error.
int moisture_read_raw(void);

// Averaged sensor output voltage in volts. Returns a negative value on error.
float moisture_read_voltage(void);