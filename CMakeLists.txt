cmake_minimum_required(VERSION 3.20)

project(VulkanLearn)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(target_cxx_std)
include(target_hide_symbols)
include(target_strict_compilation)

add_subdirectory(glad)
add_subdirectory(pvk)

set(SOME_PVK_TARGET_EXIST FALSE)
if(TARGET pvk.static)
    set(SOME_PVK_TARGET_EXIST TRUE)
endif()
if(TARGET pvk.shared)
    set(SOME_PVK_TARGET_EXIST TRUE)
endif()

if(NOT ${SOME_PVK_TARGET_EXIST})
    message(FATAL_ERROR "No pvk library found")
endif()


if (WIN32 AND NOT TARGET pvk.static)
    message(FATAL_ERROR "Linking Main with pvk.shared is not supported on Windows")
endif()

add_library(Main.objects OBJECT)
target_cxx_std(Main.objects 20)
target_strict_compilation(Main.objects)
target_link_libraries(Main.objects PRIVATE pvk.headers)
target_link_libraries(Main.objects PRIVATE pvk.symvis.emptymacro)

target_sources(Main.objects PRIVATE
    Main.cc
)

add_executable(Main)
target_sources(Main PRIVATE
    $<TARGET_OBJECTS:Main.objects>
)

if (WIN32)
    target_compile_options(Main.objects PRIVATE USE_WINDOWS_VULKAN)
    target_link_libraries(Main PRIVATE pvk.static)
    return()
endif()

if(TARGET pvk.shared)
    target_link_libraries(Main PRIVATE pvk.shared)
else()
    target_link_libraries(Main PRIVATE pvk.static)
endif()
