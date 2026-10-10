#include "mqtt.h"
#include <stdio.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "cmd/cmd_table.h"
#include "cmd/func.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "led/led.h"

// --- HiveMQ Credentials ---
// Example host: "xxxxxx.s1.eu.hivemq.cloud" (DO NOT include "mqtts://")
#define HIVEMQ_HOST     "6282eedebf8e4c00963ac946feab2c62.s1.eu.hivemq.cloud" 
#define HIVEMQ_PORT     8883
#define HIVEMQ_USER     "hivemq.webclient.17887940417135"
#define HIVEMQ_PASS     "aGku8b3naRARxn%LNLOkIgqJOsniheT2"

// --- Reconnect policy ---
#define MQTT_MAX_RETRY       7
#define MQTT_RETRY_DELAY_MS  30000

// Import the embedded root certificate address from binary symbols
extern const uint8_t hivemq_ca_pem_start[] asm("_binary_isrgrootx1_pem_start");
extern const uint8_t hivemq_ca_pem_end[]   asm("_binary_isrgrootx1_pem_end");

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static esp_timer_handle_t s_retry_timer = NULL;
static int s_retry_count = 0;

/* Retry delay elapsed: try to connect again (non-blocking, runs in esp_timer task) */
static void mqtt_retry_timer_cb(void *arg)
{
    esp_mqtt_client_reconnect(mqtt_client);
}

/* Called on every disconnect or failed connection: retry after a delay, or give up */
static void mqtt_schedule_retry(void)
{
    if (s_retry_count < MQTT_MAX_RETRY) {
        s_retry_count++;
        led_set_state(LED_STATE_MQTT_CONNECTING);
        ESP_LOGW(TAG, "MQTT disconnected, retry %d/%d in %d s",
                 s_retry_count, MQTT_MAX_RETRY, MQTT_RETRY_DELAY_MS / 1000);
        esp_timer_start_once(s_retry_timer, (uint64_t)MQTT_RETRY_DELAY_MS * 1000);
    } else {
        ESP_LOGE(TAG, "MQTT failed after %d retries, giving up", MQTT_MAX_RETRY);
        led_set_state(LED_STATE_ERROR);
    }
}

/* Register command functions (once, before the client starts) */
static void mqtt_register_commands(void)
{
    ESP_ERROR_CHECK(cmd_table_register("startpump", pump_start_cmd));
}

/* Subscribe to command topics (runs on every (re)connect) */
static void mqtt_subcribe(esp_mqtt_client_handle_t client)
{
    esp_mqtt_client_subscribe(client, "startpump", 0);
}

/* MQTT Event Handler */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    cmd_callback_t callback = NULL;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected to HiveMQ!");
            s_retry_count = 0;
            led_set_state(LED_STATE_ONLINE);
            mqtt_subcribe(client);
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_schedule_retry();
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Message Received:");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            // Dispatch to the registered command function
            callback = cmd_table_find(event->topic, event->topic_len);
            if (callback != NULL) {
                callback(event->data, event->data_len);
            } else {
                ESP_LOGW(TAG, "No command registered for this topic");
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;

        default:
            break;
    }
}

/* Start the ESP MQTT Client */
void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = NULL,
                .hostname = HIVEMQ_HOST,
                .port = HIVEMQ_PORT,
                .transport = MQTT_TRANSPORT_OVER_SSL,
            },
            .verification = {
                .certificate = (const char *)hivemq_ca_pem_start,
                .certificate_len = (size_t)(hivemq_ca_pem_end - hivemq_ca_pem_start)
            },
        },
        .credentials = {
            .username = HIVEMQ_USER,
            .authentication = {
                .password = HIVEMQ_PASS,
            },
        },
        .network = {
            // Reconnects are handled by mqtt_schedule_retry() instead
            .disable_auto_reconnect = true,
        },
    };

    mqtt_register_commands();

    const esp_timer_create_args_t retry_timer_args = {
        .callback = mqtt_retry_timer_cb,
        .name     = "mqtt_retry",
    };
    ESP_ERROR_CHECK(esp_timer_create(&retry_timer_args, &s_retry_timer));

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

}