#pragma once

void led_control(const char *data, int data_len);

// Topic: startpump, payload: "<duty 0-100>,<duration_ms>" e.g. "80,5000"
void pump_start_cmd(const char *data, int data_len);

// Topic: getmoisture, payload: ignored
// Publishes the sensor voltage in volts (e.g. "1.234") on moisture/voltage, or "error"
void moisture_read_cmd(const char *data, int data_len);