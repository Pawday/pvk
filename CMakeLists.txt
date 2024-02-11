cmake_minimum_required(VERSION 3.20)

project(VulkanLearn)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(target_cxx_std)

add_subdirectory(glad)
add_subdirectory(pvk)

add_library(Main_objects OBJECT)
target_cxx_std(Main_objects 20)
target_link_libraries(Main_objects PRIVATE pvk_headers)
target_sources(Main_objects PRIVATE
    Main.cc
)

add_executable(Main)
target_sources(Main PRIVATE
    $<TARGET_OBJECTS:Main_objects>
)

target_link_libraries(Main PRIVATE pvk_static)
