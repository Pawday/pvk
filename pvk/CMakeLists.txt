cmake_minimum_required(VERSION 3.20)

project(pvk VERSION 0.0.1)

option(PVK_USE_EXT_DEBUG_UTILS "Try load and use VK_EXT_debug_utils" OFF)
option(PVK_USE_KHR_VALIDATION_LAYER "Try load and use VK_LAYER_KHRONOS_validation" OFF)

option(PVK_ALLOCATOR_ENABLE_ALIGN_MISMATCH_DEBUG "Enable align mismatch log (some drivers do )" OFF)

set(PVK_BUILD_SHARED_DEFAULT OFF)
set(PVK_BUILD_STATIC_DEFAULT ON)
if (${BUILD_SHARED_LIBS})
    set(PVK_BUILD_SHARED_DEFAULT ON)
    set(PVK_BUILD_STATIC_DEFAULT OFF)
endif()

option(PVK_BUILD_SHARED "Build pvk shared library" ${PVK_BUILD_SHARED_DEFAULT})
option(PVK_BUILD_STATIC "Build pvk static library" ${PVK_BUILD_STATIC_DEFAULT})

function(target_pvk_options TARGET)
    target_cxx_std(${TARGET} 20)
    target_hide_symbols(${TARGET})
    target_strict_compilation(${TARGET})
endfunction()

add_subdirectory(symvis)
function(pvk_link_export TARGET)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE pvk.symvis.declspec)
    else()
        target_link_libraries(${TARGET} PRIVATE pvk.symvis.attribute)
    endif()
endfunction()

set(PVK_ANY_OF_EXTENSIONS_IN_USE FALSE)

if (PVK_USE_EXT_DEBUG_UTILS)
    set(PVK_ANY_OF_EXTENSIONS_IN_USE TRUE)
endif()

add_library(pvk.headers INTERFACE)
target_include_directories(pvk.headers INTERFACE include)

add_library(pvk.headers.internal INTERFACE)
target_include_directories(pvk.headers.internal INTERFACE include/internal)
target_link_libraries(pvk.headers.internal INTERFACE glad.headers)

add_library(pvk.allocator OBJECT allocator.cc)
set_property(TARGET pvk.allocator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(pvk.allocator PRIVATE pvk.headers pvk.headers.internal)
target_pvk_options(pvk.allocator)

if (PVK_ALLOCATOR_ENABLE_ALIGN_MISMATCH_DEBUG)
    target_compile_definitions(pvk.allocator PRIVATE PVK_ALLOCATOR_ENABLE_ALIGN_MISMATCH_DEBUG)
endif()

if(WIN32)
    target_compile_definitions(pvk.allocator PRIVATE USE_WINDOWS_ALIGNED_ALLOC)
else()
    target_compile_definitions(pvk.allocator PRIVATE GIVE_NEVIDIA_MORE_MEMORY)
endif()

add_library(pvk.objects OBJECT)
pvk_link_export(pvk.objects)
set_property(TARGET pvk.objects PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(pvk.objects PRIVATE pvk.headers pvk.headers.internal)
target_pvk_options(pvk.objects)
target_sources(pvk.objects PRIVATE
    device_context_impl.cc
    instance_impl.cc
    layer_utils.cc
    loader.cc
    log_global.cc
    log_utils_box.cc
    logger.cc
    pipeline.cc
)

if(WIN32)
    target_sources(pvk.objects PRIVATE dso_loader_win32.cc)
else()
    target_sources(pvk.objects PRIVATE dso_loader_posix.cc)
endif()

if (PVK_ANY_OF_EXTENSIONS_IN_USE)
    add_library(pvk.extensions.objects OBJECT)
    target_pvk_options(pvk.extensions.objects)
    target_link_libraries(pvk.extensions.objects PRIVATE pvk.headers pvk.headers.internal)
    target_include_directories(pvk.extensions.objects PRIVATE include/pvk)
endif()

if (PVK_USE_EXT_DEBUG_UTILS)
    include(extensions/debug_utils.cmake)
    target_compile_definitions(pvk.objects PRIVATE PVK_USE_EXT_DEBUG_UTILS)
    target_compile_definitions(pvk.extensions.objects PRIVATE PVK_USE_EXT_DEBUG_UTILS)
endif()

if (PVK_USE_KHR_VALIDATION_LAYER)
    target_compile_definitions(pvk.objects PRIVATE PVK_USE_KHR_VALIDATION_LAYER)
endif()



message(STATUS "BUILD_SHARED_LIBS is \"${BUILD_SHARED_LIBS}\"")
message(STATUS "PVK_BUILD_SHARED is ${PVK_BUILD_SHARED}")
message(STATUS "PVK_BUILD_STATIC is ${PVK_BUILD_STATIC}")

if (PVK_BUILD_SHARED)
    include(pvk_shared.cmake)
endif()

if (PVK_BUILD_STATIC)
    include(pvk_static.cmake)
endif()
