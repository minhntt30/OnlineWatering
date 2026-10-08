set(app_sources
    "main.c"
    "wifi/wifi.c"
    "mqtt/mqtt.c"
    "cmd/cmd_table.c"
    "cmd/func.c"
    "pump/pump.c"
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "." "wifi" "mqtt" "cmd" "pump"
    EMBED_TXTFILES "isrgrootx1.pem"
)