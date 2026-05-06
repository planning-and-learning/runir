add_library(core
    template_feature.cpp
)
add_library(template_feature::core ALIAS core)

set_target_properties(core PROPERTIES OUTPUT_NAME template_feature_core)

target_compile_features(core PUBLIC cxx_std_20)

target_include_directories(core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${TEMPLATE_FEATURE_INSTALL_INTERFACE_INCLUDEDIR}>
)

target_link_libraries(core
    PUBLIC
        fmt::fmt-header-only
        tyr::core
)

target_compile_definitions(core
    PUBLIC
        FMT_HEADER_ONLY
)

if(BUILD_SHARED_LIBS)
    if(APPLE)
        set(TEMPLATE_FEATURE_LIBRARY_ORIGIN "@loader_path")
    elseif(UNIX)
        set(TEMPLATE_FEATURE_LIBRARY_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(core PROPERTIES INSTALL_RPATH "${TEMPLATE_FEATURE_LIBRARY_ORIGIN}")
endif()

install(TARGETS core
    EXPORT template_featurecoreTargets
    LIBRARY DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}" COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT}
    ARCHIVE DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}" COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT}
    RUNTIME DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_BINDIR}" COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT}
    INCLUDES DESTINATION "${TEMPLATE_FEATURE_INSTALL_INTERFACE_INCLUDEDIR}"
)

install(EXPORT template_featurecoreTargets
    NAMESPACE template_feature::
    FILE "template_featurecoreTargets.cmake"
    DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}/cmake/template_feature"
    COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT}
)

export(EXPORT template_featurecoreTargets
    NAMESPACE template_feature::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/template_featurecoreTargets.cmake"
)
