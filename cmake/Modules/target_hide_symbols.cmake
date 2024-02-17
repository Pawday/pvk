function(target_hide_symbols TARGET)
    set_property(TARGET ${TARGET} PROPERTY CXX_VISIBILITY_PRESET "hidden")
    set_property(TARGET ${TARGET} PROPERTY C_VISIBILITY_PRESET "hidden")
    set_property(TARGET ${TARGET} PROPERTY VISIBILITY_INLINES_HIDDEN ON)
endfunction()
