add_library(graphs INTERFACE)
add_library(runir::graphs ALIAS graphs)

target_compile_features(graphs INTERFACE cxx_std_20)

target_include_directories(graphs
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${RUNIR_INSTALL_INTERFACE_INCLUDEDIR}>
)

target_link_libraries(graphs
    INTERFACE
        tyr::core
)

install(TARGETS graphs
    EXPORT runirgraphsTargets
    COMPONENT ${RUNIR_INSTALL_COMPONENT}
    INCLUDES DESTINATION "${RUNIR_INSTALL_INTERFACE_INCLUDEDIR}"
)

install(EXPORT runirgraphsTargets
    NAMESPACE runir::
    FILE "runirgraphsTargets.cmake"
    DESTINATION "${RUNIR_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    COMPONENT ${RUNIR_INSTALL_COMPONENT}
)

export(EXPORT runirgraphsTargets
    NAMESPACE runir::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/runirgraphsTargets.cmake"
)
