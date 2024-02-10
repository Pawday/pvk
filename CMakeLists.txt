cmake_minimum_required(VERSION 3.20)


function(target_cxx_std EXE_TARGET VERSION)
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_STANDARD ${VERSION})
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_EXTENSIONS OFF)
endfunction()

project(VulkanLearn)


add_library(glad OBJECT glad/src/vulkan.c)
target_include_directories(glad PUBLIC glad/include)

add_library(vk_allocator OBJECT vk_allocator.cc)
target_include_directories(vk_allocator PRIVATE include)
target_cxx_std(vk_allocator 20)
target_link_libraries(vk_allocator PRIVATE glad)

add_executable(Main)
target_include_directories(Main PRIVATE include)
target_link_libraries(Main PRIVATE glad)
target_cxx_std(Main 20)
target_sources(Main PRIVATE
    Main.cc
    vk_context.cc
    vk_loader.cc
)

target_link_libraries(Main PRIVATE vk_allocator)

if(WIN32)
    target_sources(Main PRIVATE
        dso_loader_win32.cc
    )
else()
    target_sources(Main PRIVATE
        dso_loader_linux.cc
    )
endif()


if(WIN32)
    target_compile_definitions(vk_allocator PRIVATE USE_WINDOWS_ALIGNED_ALLOC)
else()
    target_compile_definitions(vk_allocator PRIVATE GIVE_NEVIDIA_MORE_MEMORY)
endif()


