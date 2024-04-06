cmake_minimum_required(VERSION 3.20)


add_library(pvk.symvis.attribute INTERFACE)
target_include_directories(pvk.symvis.attribute INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/attribute/include>)

add_library(pvk.symvis.emptymacro INTERFACE)
target_include_directories(pvk.symvis.emptymacro INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/emptymacro/include>)

if (WIN32)
    add_library(pvk.symvis.declspec INTERFACE)
    target_include_directories(pvk.symvis.declspec INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/declspec/include>)

    add_library(pvk.symvis.declspec.import INTERFACE)
    target_include_directories(pvk.symvis.declspec.import INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/declspec_import/include>)
endif()
