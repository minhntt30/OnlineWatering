
#pragma once
#include <stdint.h>
#include "esp_err.h"

#define CMD_TABLE_MAX_ENTRIES 16

// Called from the MQTT task: keep it short and non-blocking.
// 'data' is NOT null-terminated, always use data_len.
typedef void (*cmd_callback_t)(const char *data, int data_len);

// Hash the topic and store it with its callback (call before mqtt_app_start)
esp_err_t cmd_table_register(const char *topic, cmd_callback_t callback);

// Look up a callback by topic. 'topic' is NOT null-terminated. Returns NULL if not found.
cmd_callback_t cmd_table_find(const char *topic, int topic_len);