#pragma once
#include "esp_err.h"

// Initializes Wi-Fi in station mode and blocks until an IP is obtained.
// Returns ESP_OK when connected, ESP_FAIL if all retries were used up.
esp_err_t wifi_init_sta(void);