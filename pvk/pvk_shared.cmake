add_library(pvk_shared SHARED)

# ========= ARTEFACT NAME ========= #
set_property(TARGET pvk_shared PROPERTY OUTPUT_NAME "pvk")





# ========= ARTEFACT SOURCES ========= #
target_sources(pvk_shared PRIVATE
    $<TARGET_OBJECTS:pvk_allocator>
    $<TARGET_OBJECTS:pvk_objects>
    $<TARGET_OBJECTS:glad_objects>
)

# ========= ARTEFACT VK EXTENSIONS ========= #
if (PVK_ANY_OF_EXTENSIONS_IN_USE)
    target_sources(pvk_shared PRIVATE $<TARGET_OBJECTS:pvk_extensions_objects>)
endif()


# ========= ARTEFACT SYMBOL VISIBILITY ========= #
if(WIN32)
    target_link_libraries(pvk_shared INTERFACE pvk_symvis_declspec_import)
else()
    target_link_libraries(pvk_shared INTERFACE pvk_symvis_emptymacro)
endif()
