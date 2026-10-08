#pragma once

void led_control(const char *data, int data_len);

// Topic: startpump, payload: "<duty 0-100>,<duration_ms>" e.g. "80,5000"
void pump_start_cmd(const char *data, int data_len);