#pragma once

// Creates and starts the HiveMQ MQTT client (call after Wi-Fi is connected)
void mqtt_app_start(void);

// Publishes a text message (QoS 0, not retained). Returns false if it could not be sent.
bool mqtt_publish_text(const char *topic, const char *payload);