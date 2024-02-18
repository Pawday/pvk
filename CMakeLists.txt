cmake_minimum_required(VERSION 3.20)

project(VulkanLearn)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(target_cxx_std)
include(target_hide_symbols)
include(target_strict_compilation)

add_subdirectory(glad)
add_subdirectory(pvk)

set(SOME_PVK_TARGET_EXIST FALSE)
if(TARGET pvk_static)
    set(SOME_PVK_TARGET_EXIST TRUE)
endif()
if(TARGET pvk_shared)
    set(SOME_PVK_TARGET_EXIST TRUE)
endif()

if(NOT ${SOME_PVK_TARGET_EXIST})
    message(WARNING "No pvk library found")
    return()
endif()


if (WIN32 AND NOT TARGET pvk_static)
    message(WARNING "Linking Main with pvk_shared is not supported on Windows")
    return()
endif()

add_library(Main_objects OBJECT)
target_cxx_std(Main_objects 20)
target_strict_compilation(Main_objects)
target_link_libraries(Main_objects PRIVATE pvk_headers)
target_link_libraries(Main_objects PRIVATE pvk_symvis_emptymacro)

target_sources(Main_objects PRIVATE
    Main.cc
)

add_executable(Main)
target_sources(Main PRIVATE
    $<TARGET_OBJECTS:Main_objects>
)

if (WIN32)
    target_link_libraries(Main PRIVATE pvk_static)
    return()
endif()

if(TARGET pvk_shared)
    target_link_libraries(Main PRIVATE pvk_shared)
else()
    target_link_libraries(Main PRIVATE pvk_static)
endif()
