add_library(bsp_uart STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp_uart.cpp
)
target_include_directories(bsp_uart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bsp_uart PUBLIC zephyr_interface)
