cmake_minimum_required(VERSION 3.16)

# Porta → firmware → UI (arquitetura unidirecional preservada)
set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/../runesos-ui
    ${CMAKE_CURRENT_LIST_DIR}/../firmware
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options(-Wno-error=cast-function-type -Wno-cast-function-type)
project(lv_port_esp32s3)

