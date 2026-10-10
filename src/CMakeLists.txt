set(app_sources
    "main.c"
    "wifi/wifi.c"
    "mqtt/mqtt.c"
    "cmd/cmd_table.c"
    "cmd/func.c"
    "pump/pump.c"
    "watchdog/watchdog.c"
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "." "wifi" "mqtt" "cmd" "pump" "watchdog"
    EMBED_TXTFILES "isrgrootx1.pem"
)