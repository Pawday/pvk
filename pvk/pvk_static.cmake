add_library(pvk_static STATIC)

# ========= ARTEFACT NAME ========= #
set(PVK_STATIC_LIB_ARTEFACT_NAME "pvk")
if(WIN32)
    set(PVK_STATIC_LIB_ARTEFACT_NAME "pvk_static")
endif()
set_property(TARGET pvk_static PROPERTY OUTPUT_NAME ${PVK_STATIC_LIB_ARTEFACT_NAME})

# ========= ARTEFACT SOURCES ========= #
target_sources(pvk_static PRIVATE
    $<TARGET_OBJECTS:pvk_allocator>
    $<TARGET_OBJECTS:pvk_objects>
    $<TARGET_OBJECTS:glad_objects>
)

# ========= ARTEFACT VK EXTENSIONS ========= #
if (PVK_ANY_OF_EXTENSIONS_IN_USE)
    target_sources(pvk_static PRIVATE $<TARGET_OBJECTS:pvk_extensions_objects>)
endif()


# ========= ARTEFACT SYMBOL VISIBILITY ========= #
target_link_libraries(pvk_static INTERFACE pvk_symvis_emptymacro)




