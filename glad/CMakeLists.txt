cmake_minimum_required(VERSION 3.20)

add_library(glad_headers INTERFACE)
target_include_directories(glad_headers INTERFACE include)
add_library(glad_objects OBJECT src/vulkan.c)
set_property(TARGET glad_objects PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(glad_objects PRIVATE glad_headers)
