cmake_minimum_required(VERSION 3.20)

option(PVK_USE_EXT_DEBUG_UTILS "Try load and use VK_EXT_debug_utils" OFF)
option(PVK_USE_KHR_VALIDATION_LAYER "Try load and use VK_LAYER_KHRONOS_validation" OFF)

add_subdirectory(symvis)

function(pvk_link_export TARGET)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE pvk_symvis_declspec)
    else()
        target_link_libraries(${TARGET} PRIVATE pvk_symvis_attribute)
    endif()
endfunction()

set(PVK_ANY_OF_EXTENSIONS_IN_USE FALSE)

if (PVK_USE_EXT_DEBUG_UTILS)
    set(PVK_ANY_OF_EXTENSIONS_IN_USE TRUE)
endif()

add_library(pvk_headers INTERFACE)
target_include_directories(pvk_headers INTERFACE include)
target_link_libraries(pvk_headers INTERFACE glad_headers)

add_library(pvk_allocator OBJECT vk_allocator.cc)
set_property(TARGET pvk_allocator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_cxx_std(pvk_allocator 20)
target_hide_symbols(pvk_allocator)
target_link_libraries(pvk_allocator PRIVATE pvk_headers)
if(WIN32)
    target_compile_definitions(pvk_allocator PRIVATE USE_WINDOWS_ALIGNED_ALLOC)
else()
    target_compile_definitions(pvk_allocator PRIVATE GIVE_NEVIDIA_MORE_MEMORY)
endif()

add_library(pvk_objects OBJECT)
set_property(TARGET pvk_objects PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(pvk_objects PRIVATE pvk_headers)
target_cxx_std(pvk_objects 20)
target_hide_symbols(pvk_objects)
pvk_link_export(pvk_objects)
target_sources(pvk_objects PRIVATE
    vk_context.cc
    vk_loader.cc
    vk_context.cc
    vk_loader.cc
    log.cc
)
if(WIN32)
    target_sources(pvk_objects PRIVATE dso_loader_win32.cc)
else()
    target_sources(pvk_objects PRIVATE dso_loader_posix.cc)
endif()

if (PVK_ANY_OF_EXTENSIONS_IN_USE)
    add_library(pvk_extensions_objects OBJECT)
    target_hide_symbols(pvk_extensions_objects)
    target_cxx_std(pvk_extensions_objects 20)
    target_link_libraries(pvk_extensions_objects PRIVATE pvk_headers)

    message(WARNING "TODO: remove target_include_directories with .cc file acces")
    target_include_directories(pvk_extensions_objects PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if (PVK_USE_EXT_DEBUG_UTILS)
    include(extensions/debug_utils.cmake)
    target_compile_definitions(pvk_objects PRIVATE PVK_USE_EXT_DEBUG_UTILS)
    target_compile_definitions(pvk_extensions_objects PRIVATE PVK_USE_EXT_DEBUG_UTILS)
endif()

if (PVK_USE_KHR_VALIDATION_LAYER)
    target_compile_definitions(pvk_objects PRIVATE PVK_USE_KHR_VALIDATION_LAYER)
endif()

set(PVK_BUILD_SHARED_DEFAULT OFF)
set(PVK_BUILD_STATIC_DEFAULT ON)
if (${BUILD_SHARED_LIBS})
    set(PVK_BUILD_SHARED_DEFAULT ON)
    set(PVK_BUILD_STATIC_DEFAULT OFF)
endif()

option(PVK_BUILD_SHARED "Build pvk shared library" ${PVK_BUILD_SHARED_DEFAULT})
option(PVK_BUILD_STATIC "Build pvk static library" ${PVK_BUILD_STATIC_DEFAULT})

message(STATUS "BUILD_SHARED_LIBS is \"${BUILD_SHARED_LIBS}\"")
message(STATUS "PVK_BUILD_SHARED is ${PVK_BUILD_SHARED}")
message(STATUS "PVK_BUILD_STATIC is ${PVK_BUILD_STATIC}")

if (PVK_BUILD_SHARED)
    include(pvk_shared.cmake)
endif()

if (PVK_BUILD_STATIC)
    include(pvk_static.cmake)
endif()
