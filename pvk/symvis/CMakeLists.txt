cmake_minimum_required(VERSION 3.20)


add_library(pvk_symvis_attribute INTERFACE)
target_include_directories(pvk_symvis_attribute INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/attribute/include>)

add_library(pvk_symvis_emptymacro INTERFACE)
target_include_directories(pvk_symvis_emptymacro INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/noexport/include>)


if (WIN32)
    add_library(pvk_symvis_declspec INTERFACE)
    target_include_directories(pvk_symvis_declspec INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/declspec/include>)

    add_library(pvk_symvis_declspec_import INTERFACE)
    target_include_directories(pvk_symvis_declspec_import INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/declspec_import/include>)
endif()
