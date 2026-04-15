# controller 容器：自动扫描所有包含 CMakeLists.txt 的子目录
set(CONTROLLER_TARGETS "")
file(GLOB CTRL_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(subdir ${CTRL_SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt)
        add_subdirectory(${subdir})
        if(TARGET ctrl_${subdir})
            list(APPEND CONTROLLER_TARGETS ctrl_${subdir})
        endif()
    endif()
endforeach()
set(CONTROLLER_TARGETS ${CONTROLLER_TARGETS} PARENT_SCOPE)
