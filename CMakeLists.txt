cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(mqtt_motor C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Caso seja a Raspberry Pi Pico W
set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(mqtt_motor main.c)

pico_enable_stdio_usb(mqtt_motor 1)
pico_enable_stdio_uart(mqtt_motor 0)

target_include_directories(mqtt_motor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(mqtt_motor
        tinyusb_board
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        pico_lwip_mqtt
        hardware_pwm
    )
pico_add_extra_outputs(mqtt_motor)
