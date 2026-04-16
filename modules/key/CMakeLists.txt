add_library(mod_key INTERFACE)
target_include_directories(mod_key INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mod_key INTERFACE bsp_gpio)
