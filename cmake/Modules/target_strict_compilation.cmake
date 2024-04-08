function(target_strict_compilation TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE "/W1" "/WX")
        return()
    endif()

    target_compile_options(${TARGET} PRIVATE "-Wall" "-Wextra" "-Wshadow" "-Werror")
endfunction()
