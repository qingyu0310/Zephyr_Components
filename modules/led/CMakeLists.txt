add_library(mod_led INTERFACE)
target_include_directories(mod_led INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mod_led INTERFACE bsp_gpio)
