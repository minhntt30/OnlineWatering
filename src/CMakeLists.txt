# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
    SRCS "main.c" "wifi/wifi.c" "mqtt/mqtt.c" "cmd/cmd_table.c" "cmd/func.c"
    INCLUDE_DIRS "." "wifi" "mqtt" "cmd"
    EMBED_TXTFILES "isrgrootx1.pem"
)