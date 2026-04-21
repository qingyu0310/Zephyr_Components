cmake_minimum_required(VERSION 3.20.0)

if(DEFINED BOARD)
    # 使用命令行指定的 BOARD
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/stm32f4_disco.overlay")
    set(BOARD stm32f4_disco)
else()
    set(BOARD ek_ra2l1)
endif()

if(BOARD MATCHES "^ek_ra2l1")
    set(BOARD_FLASH_RUNNER pyocd)
    set(BOARD_DEBUG_RUNNER pyocd)
    set(BOARD_RUNNER_ARGS_pyocd "--frequency=1000000")
elseif(BOARD MATCHES "^stm32f4_disco")
    set(BOARD_FLASH_RUNNER openocd)
    set(BOARD_DEBUG_RUNNER openocd)
  set(BOARD_RUNNER_ARGS_openocd "--config=${CMAKE_CURRENT_SOURCE_DIR}/config/openocd_dap.cfg")
endif()

# 自动收集各组件目录下的 module.conf，并在 Zephyr 配置阶段一并合入。
set(AUTO_EXTRA_CONF_FILES "")
function(collect_component_conf container_dir conf_name)
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${container_dir})
    return()
  endif()

  file(GLOB CHILDREN RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${container_dir} ${CMAKE_CURRENT_SOURCE_DIR}/${container_dir}/*)
  foreach(child ${CHILDREN})
    set(child_dir ${CMAKE_CURRENT_SOURCE_DIR}/${container_dir}/${child})
    set(conf_file ${child_dir}/${conf_name})
    if(IS_DIRECTORY ${child_dir} AND EXISTS ${child_dir}/CMakeLists.txt AND EXISTS ${conf_file})
      list(APPEND AUTO_EXTRA_CONF_FILES ${conf_file})
    endif()
  endforeach()

  set(AUTO_EXTRA_CONF_FILES ${AUTO_EXTRA_CONF_FILES} PARENT_SCOPE)
endfunction()

collect_component_conf(bsp module.conf)
collect_component_conf(modules module.conf)
collect_component_conf(controller module.conf)

if(AUTO_EXTRA_CONF_FILES)
  set(EXTRA_CONF_FILE "${AUTO_EXTRA_CONF_FILES}" CACHE STRING "Extra Zephyr conf files" FORCE)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

# 源文件目录列表（在此添加新的源文件目录，会自动递归搜索 .c 文件）
set(APP_SOURCE_DIRS
    apps
    communication
    src
    thread
    # src/xxx
)

# 头文件目录列表（在此添加新的头文件目录，会自动递归搜索子目录）
set(APP_INCLUDE_DIRS
    apps
    communication
    src
    thread
    topic
    include
    # src/xxx
)

foreach(dir ${APP_SOURCE_DIRS})
    file(GLOB_RECURSE DIR_SOURCES ${dir}/*.c ${dir}/*.cpp)
    list(APPEND APP_SOURCES ${DIR_SOURCES})
endforeach()

# 递归搜索头文件子目录
set(ALL_INCLUDE_DIRS)
foreach(dir ${APP_INCLUDE_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        list(APPEND ALL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hpp)
        foreach(header ${HEADERS})
            get_filename_component(header_dir ${header} DIRECTORY)
            list(APPEND ALL_INCLUDE_DIRS ${header_dir})
        endforeach()
    endif()
endforeach()
list(REMOVE_DUPLICATES ALL_INCLUDE_DIRS)

target_sources(app PRIVATE ${APP_SOURCES})
target_include_directories(app PRIVATE ${ALL_INCLUDE_DIRS})

# bsp/ 和 modules/ 通过 mod_init.py 从 Zephyr_Components 仓库拉取
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bsp)
  add_subdirectory(bsp)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/modules)
  add_subdirectory(modules)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/controller)
  add_subdirectory(controller)
endif()

# 自动链接所有扫描到的 bsp、模块和控制器
foreach(bsp ${BSP_TARGETS})
  target_link_libraries(app PRIVATE ${bsp})
endforeach()
foreach(mod ${MODULE_TARGETS})
  target_link_libraries(app PRIVATE ${mod})
endforeach()
foreach(ctrl ${CONTROLLER_TARGETS})
  target_link_libraries(app PRIVATE ${ctrl})
endforeach()












