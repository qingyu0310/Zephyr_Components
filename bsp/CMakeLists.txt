# bsp 容器：自动扫描所有包含 CMakeLists.txt 的子目录
set(BSP_TARGETS "")
file(GLOB BSP_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(subdir ${BSP_SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt)
        add_subdirectory(${subdir})
        list(APPEND BSP_TARGETS ${subdir})
    endif()
endforeach()
set(BSP_TARGETS ${BSP_TARGETS} PARENT_SCOPE)
