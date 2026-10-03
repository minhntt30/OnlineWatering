
#include "cmd_table.h"
#include <string.h>
#include "esp_log.h"

typedef struct {
    uint32_t topic_hash;
    cmd_callback_t callback;
} cmd_entry_t;

static const char *TAG = "CMD_TABLE";
static cmd_entry_t s_table[CMD_TABLE_MAX_ENTRIES];
static int s_count = 0;

// FNV-1a 32-bit hash
static uint32_t hash_topic(const char *topic, size_t len)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)topic[i];
        hash *= 16777619u;
    }
    return hash;
}

esp_err_t cmd_table_register(const char *topic, cmd_callback_t callback)
{
    if (topic == NULL || callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_count >= CMD_TABLE_MAX_ENTRIES) {
        ESP_LOGE(TAG, "Table full, cannot register %s", topic);
        return ESP_ERR_NO_MEM;
    }

    uint32_t hash = hash_topic(topic, strlen(topic));

    // Reject duplicates and hash collisions
    for (int i = 0; i < s_count; i++) {
        if (s_table[i].topic_hash == hash) {
            ESP_LOGE(TAG, "Hash already used, cannot register %s", topic);
            return ESP_ERR_INVALID_STATE;
        }
    }

    s_table[s_count].topic_hash = hash;
    s_table[s_count].callback = callback;
    s_count++;
    return ESP_OK;
}

cmd_callback_t cmd_table_find(const char *topic, int topic_len)
{
    if (topic == NULL || topic_len <= 0) {
        return NULL;
    }

    uint32_t hash = hash_topic(topic, (size_t)topic_len);
    for (int i = 0; i < s_count; i++) {
        if (s_table[i].topic_hash == hash) {
            return s_table[i].callback;
        }
    }
    return NULL;
}