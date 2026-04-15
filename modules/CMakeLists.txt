# modules 容器：自动扫描所有包含 CMakeLists.txt 的子目录
set(MODULE_TARGETS "")
file(GLOB MOD_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(subdir ${MOD_SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt)
        add_subdirectory(${subdir})
        if(TARGET mod_${subdir})
            list(APPEND MODULE_TARGETS mod_${subdir})
        endif()
    endif()
endforeach()
set(MODULE_TARGETS ${MODULE_TARGETS} PARENT_SCOPE)
