add_library(mod_servo INTERFACE)
target_include_directories(mod_servo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mod_servo INTERFACE bsp_pwm)
