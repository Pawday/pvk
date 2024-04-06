add_library(pvk.shared SHARED)

# ========= ARTEFACT NAME ========= #
set_property(TARGET pvk.shared PROPERTY OUTPUT_NAME "pvk")





# ========= ARTEFACT SOURCES ========= #
target_sources(pvk.shared PRIVATE
    $<TARGET_OBJECTS:pvk.allocator>
    $<TARGET_OBJECTS:pvk.objects>
    $<TARGET_OBJECTS:glad.objects>
)

# ========= ARTEFACT VK EXTENSIONS ========= #
if (PVK_ANY_OF_EXTENSIONS_IN_USE)
    target_sources(pvk.shared PRIVATE $<TARGET_OBJECTS:pvk.extensions.objects>)
endif()


# ========= ARTEFACT SYMBOL VISIBILITY ========= #
if(WIN32)
    target_link_libraries(pvk.shared INTERFACE pvk.symvis.declspec.import)
else()
    target_link_libraries(pvk.shared INTERFACE pvk.symvis.emptymacro)
endif()

# ========= ARTEFACT VERSION ========= #
set_property(TARGET pvk.shared PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET pvk.shared PROPERTY SOVERSION ${PROJECT_VERSION_MAJOR})
