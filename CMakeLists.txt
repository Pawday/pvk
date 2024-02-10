cmake_minimum_required(VERSION 3.20)


function(target_cxx_std EXE_TARGET VERSION)
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_STANDARD ${VERSION})
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${EXE_TARGET} PROPERTY CXX_EXTENSIONS OFF)
endfunction()

project(VulkanLearn)


add_executable(Main)

target_include_directories(Main PRIVATE glad/include)
target_cxx_std(Main 20)

target_sources(Main PRIVATE
    Main.cc
    glad/src/vulkan.c
    vk_context.cc
    vk_allocator.cc
    vk_loader.cc
)

if(WIN32)
    target_sources(Main PRIVATE
        dso_loader_win32.cc
    )
    target_compile_definitions(Main PRIVATE USE_WINDOWS_ALIGNED_ALLOC)
else()
    target_sources(Main PRIVATE
        dso_loader_linux.cc
    )
endif()


