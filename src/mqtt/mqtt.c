#include "mqtt.h"
#include <stdio.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "cmd/cmd_table.h"
#include "cmd/func.h"
#include "esp_err.h"

// --- HiveMQ Credentials ---
// Example host: "xxxxxx.s1.eu.hivemq.cloud" (DO NOT include "mqtts://")
#define HIVEMQ_HOST     "6282eedebf8e4c00963ac946feab2c62.s1.eu.hivemq.cloud" 
#define HIVEMQ_PORT     8883
#define HIVEMQ_USER     "hivemq.webclient.17887940417135"
#define HIVEMQ_PASS     "aGku8b3naRARxn%LNLOkIgqJOsniheT2"

// Import the embedded root certificate address from binary symbols
extern const uint8_t hivemq_ca_pem_start[] asm("_binary_isrgrootx1_pem_start");
extern const uint8_t hivemq_ca_pem_end[]   asm("_binary_isrgrootx1_pem_end");

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t mqtt_client = NULL;


static void mqtt_subcribe(esp_mqtt_client_handle_t client)
{
    esp_mqtt_client_subscribe(client, "startpump", 0);
    ESP_ERROR_CHECK(cmd_table_register("startpump", pump_start_cmd));
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
            mqtt_subcribe(client);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT Disconnected");
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
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

}