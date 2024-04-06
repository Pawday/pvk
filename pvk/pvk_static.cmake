add_library(pvk.static STATIC)

# ========= ARTEFACT NAME ========= #
set(PVK_STATIC_LIB_ARTEFACT_NAME "pvk")
if(WIN32)
    set(PVK_STATIC_LIB_ARTEFACT_NAME "pvk_static")
endif()
set_property(TARGET pvk.static PROPERTY OUTPUT_NAME ${PVK_STATIC_LIB_ARTEFACT_NAME})

# ========= ARTEFACT SOURCES ========= #
target_sources(pvk.static PRIVATE
    $<TARGET_OBJECTS:pvk.allocator>
    $<TARGET_OBJECTS:pvk.objects>
    $<TARGET_OBJECTS:glad.objects>
)

# ========= ARTEFACT VK EXTENSIONS ========= #
if (PVK_ANY_OF_EXTENSIONS_IN_USE)
    target_sources(pvk.static PRIVATE $<TARGET_OBJECTS:pvk.extensions.objects>)
endif()


# ========= ARTEFACT SYMBOL VISIBILITY ========= #
target_link_libraries(pvk.static INTERFACE pvk.symvis.emptymacro)





# ========= ARTEFACT VERSION ========= #
set_property(TARGET pvk.static PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET pvk.static PROPERTY SOVERSION ${PROJECT_VERSION_MAJOR})
