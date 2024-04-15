cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
project(Dual_LED_Test)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(Dual_LED_Test Dual_LED_Test.cpp)

pico_generate_pio_header(${PROJECT_NAME}
        ${CMAKE_CURRENT_LIST_DIR}/debounce.pio
)

pico_generate_pio_header(${PROJECT_NAME}
        ${CMAKE_CURRENT_LIST_DIR}/blink.pio
)

target_link_libraries(Dual_LED_Test pico_stdlib hardware_pio)

#pico_enable_stdio_usb(Dual_LED_Test 1)
#pico_enable_stdio_uart(Dual_LED_Test 0)

pico_add_extra_outputs(Dual_LED_Test)