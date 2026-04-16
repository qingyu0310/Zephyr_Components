add_library(bsp_pwm STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp_pwm.cpp
)
target_include_directories(bsp_pwm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bsp_pwm PUBLIC zephyr_interface)
