cmake_minimum_required(VERSION 3.20)

function(target_cxx_std EXE_TARGET VERSION)
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_STANDARD ${VERSION})
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_EXTENSIONS OFF)
endfunction()

project(VulkanLearn)

add_library(glad_headers INTERFACE glad/src/vulkan.c)
target_include_directories(glad_headers INTERFACE glad/include)

add_library(pvk_headers INTERFACE)
target_include_directories(pvk_headers INTERFACE include)
target_link_libraries(pvk_headers INTERFACE glad_headers)

add_library(glad OBJECT glad/src/vulkan.c)
set_property(TARGET glad PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(glad PRIVATE glad_headers)

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
    target_sources(pvk_objects PRIVATE dso_loader_linux.cc)
endif()

add_library(Main_objects OBJECT
    Main.cc
)
target_cxx_std(Main_objects 20)
target_link_libraries(Main_objects PRIVATE pvk_headers)

add_executable(Main)
target_sources(Main PRIVATE
    $<TARGET_OBJECTS:Main_objects>
    $<TARGET_OBJECTS:pvk_allocator>
    $<TARGET_OBJECTS:pvk_objects>
    $<TARGET_OBJECTS:glad>
)


#target_link_libraries(Main PRIVATE pvk_shared)
