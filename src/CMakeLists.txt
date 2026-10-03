# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# idf_component_register(
#     SRCS "main.c"
#     INCLUDE_DIRS "."
#     EMBED_TXTFILES "isrgrootx1.pem"
#     PRIV_REQUIRES mqtt nvs_flash esp_wifi esp_event
# )

idf_component_register(
    SRCS "main.c" "wifi/wifi.c" "mqtt/mqtt.c"
    INCLUDE_DIRS "." "wifi" "mqtt"
    EMBED_TXTFILES "isrgrootx1.pem"
)