add_library(mod_remote_fs STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/fs_i6.cpp
)
target_include_directories(mod_remote_fs PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/topic
)
target_link_libraries(mod_remote_fs PUBLIC bsp_uart bsp_pwm zephyr_interface)
