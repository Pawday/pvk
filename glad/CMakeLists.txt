cmake_minimum_required(VERSION 3.20)

add_library(glad.headers INTERFACE)
target_include_directories(glad.headers INTERFACE include)
add_library(glad.objects OBJECT src/vulkan.c)
target_hide_symbols(glad.objects)
set_property(TARGET glad.objects PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(glad.objects PRIVATE glad.headers)
