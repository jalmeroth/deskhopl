cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# define some vars required for pico-sdk
set(PICO_BOARD "pico")
set(PICO_COMPILER "pico_arm_gcc")
set(PICO_PLATFORM "rp2040")
set(PICO_PIO_USB_PATH $ENV{PICO_PIO_USB_PATH})
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(PICO_TINYUSB_PATH $ENV{PICO_TINYUSB_PATH})

# uncomment for debugging
set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_BUILD_TYPE "Debug")
# set(LOG 3)
option( DH_DEBUG "Enable Debug builds" OFF )

# FIXME: if clang is installed, discovery of compiler fails on some systems
set(CMAKE_TOOLCHAIN_FILE "${PICO_SDK_PATH}/cmake/preload/toolchains/${PICO_COMPILER}.cmake")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Ofast -Wall -mcpu=cortex-m0plus -mtune=cortex-m0plus -funroll-loops"
)
if(DH_DEBUG)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDH_DEBUG=1")
endif()
message(CMAKE_C_FLAGS="${CMAKE_C_FLAGS}")

project(deskhopl_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_library(pico_pio_usb STATIC
    ${PICO_PIO_USB_PATH}/pio_usb.c
    ${PICO_PIO_USB_PATH}/pio_usb_host.c
    ${PICO_PIO_USB_PATH}/usb_crc.c
)
pico_generate_pio_header(pico_pio_usb ${PICO_PIO_USB_PATH}/usb_tx.pio)
pico_generate_pio_header(pico_pio_usb ${PICO_PIO_USB_PATH}/usb_rx.pio)

target_link_libraries(pico_pio_usb PRIVATE
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_dma
)
target_include_directories(pico_pio_usb PRIVATE ${PICO_PIO_USB_PATH})

set(binaries board_A board_B)

foreach(board_role RANGE 0 1)
    list (GET binaries ${board_role} binary)

    add_executable(${binary})

    target_compile_definitions(${binary} PRIVATE
        BOARD_ROLE=${board_role}
        PIO_USB_DP_PIN_DEFAULT=14
    )

    target_sources(${binary} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/actions.c
        ${CMAKE_CURRENT_LIST_DIR}/handlers.c
        ${CMAKE_CURRENT_LIST_DIR}/keyboard.c
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/setup.c
        ${CMAKE_CURRENT_LIST_DIR}/tusb_d.c
        ${CMAKE_CURRENT_LIST_DIR}/tusb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/tusb_h.c
        ${CMAKE_CURRENT_LIST_DIR}/uart.c
        ${CMAKE_CURRENT_LIST_DIR}/utils.c
        ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
    )

    # Make sure TinyUSB can find tusb_config.h
    target_include_directories(${binary} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_PIO_USB_PATH}
    )

    # In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_host
    # for TinyUSB device support and tinyusb_board for the additional board support library used by the example
    target_link_libraries(${binary} PUBLIC
        hardware_pio
        pico_multicore
        pico_pio_usb 
        pico_stdlib
        tinyusb_device
        tinyusb_host
    )

    # create map/bin/hex/uf2 file etc.
    # pico_add_extra_outputs(${binary})

    target_link_options(${binary} PRIVATE
      -Xlinker
      --print-memory-usage
    )

endforeach()
