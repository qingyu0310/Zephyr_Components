add_library(mod_motor_pwm INTERFACE)
target_include_directories(mod_motor_pwm INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(mod_motor_pwm INTERFACE bsp_pwm)
