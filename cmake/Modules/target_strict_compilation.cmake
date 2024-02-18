function(target_strict_compilation TARGET)
    target_compile_options(${TARGET} PRIVATE "-Wall" "-Wextra" "-Wshadow")
endfunction()
