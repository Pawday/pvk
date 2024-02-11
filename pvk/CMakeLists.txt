cmake_minimum_required(VERSION 3.20)

add_library(pvk_headers INTERFACE)
target_include_directories(pvk_headers INTERFACE include)
target_link_libraries(pvk_headers INTERFACE glad_headers)

add_library(pvk_allocator OBJECT vk_allocator.cc)
set_property(TARGET pvk_allocator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_cxx_std(pvk_allocator 20)
target_link_libraries(pvk_allocator PRIVATE pvk_headers)
if(WIN32)
    target_compile_definitions(pvk_allocator PRIVATE USE_WINDOWS_ALIGNED_ALLOC)
else()
    target_compile_definitions(pvk_allocator PRIVATE GIVE_NEVIDIA_MORE_MEMORY)
endif()

add_library(pvk_objects OBJECT)
set_property(TARGET pvk_objects PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(pvk_objects PUBLIC pvk_headers)
target_cxx_std(pvk_objects 20)
target_sources(pvk_objects PRIVATE
    vk_context.cc
    vk_loader.cc
    vk_context.cc
    vk_loader.cc
)
if(WIN32)
    target_sources(pvk_objects PRIVATE dso_loader_win32.cc)
else()
    target_sources(pvk_objects PRIVATE dso_loader_posix.cc)
endif()

add_library(pvk_shared SHARED)
set_property(TARGET pvk_shared PROPERTY OUTPUT_NAME "pvk")

add_library(pvk_static STATIC)
if(WIN32)
    set_property(TARGET pvk_static PROPERTY OUTPUT_NAME "pvk_static")
else()
    set_property(TARGET pvk_static PROPERTY OUTPUT_NAME "pvk")
endif()

target_sources(pvk_static PRIVATE
    $<TARGET_OBJECTS:pvk_allocator>
    $<TARGET_OBJECTS:pvk_objects>
    $<TARGET_OBJECTS:glad_objects>
)

target_sources(pvk_shared PRIVATE
    $<TARGET_OBJECTS:pvk_allocator>
    $<TARGET_OBJECTS:pvk_objects>
    $<TARGET_OBJECTS:glad_objects>
)
