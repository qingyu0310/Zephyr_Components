add_library(bsp_gpio STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp_gpio.cpp
)
target_include_directories(bsp_gpio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bsp_gpio PUBLIC zephyr_interface)
