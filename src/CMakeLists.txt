add_library(core
    runir.cpp
)
add_library(runir::core ALIAS core)

set_target_properties(core PROPERTIES OUTPUT_NAME runir_core)

target_compile_features(core PUBLIC cxx_std_20)

target_include_directories(core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${RUNIR_INSTALL_INTERFACE_INCLUDEDIR}>
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
        set(RUNIR_LIBRARY_ORIGIN "@loader_path")
    elseif(UNIX)
        set(RUNIR_LIBRARY_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(core PROPERTIES INSTALL_RPATH "${RUNIR_LIBRARY_ORIGIN}")
endif()

install(TARGETS core
    EXPORT runircoreTargets
    LIBRARY DESTINATION "${RUNIR_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}" COMPONENT ${RUNIR_INSTALL_COMPONENT}
    ARCHIVE DESTINATION "${RUNIR_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}" COMPONENT ${RUNIR_INSTALL_COMPONENT}
    RUNTIME DESTINATION "${RUNIR_INSTALL_PREFIX}${CMAKE_INSTALL_BINDIR}" COMPONENT ${RUNIR_INSTALL_COMPONENT}
    INCLUDES DESTINATION "${RUNIR_INSTALL_INTERFACE_INCLUDEDIR}"
)

install(EXPORT runircoreTargets
    NAMESPACE runir::
    FILE "runircoreTargets.cmake"
    DESTINATION "${RUNIR_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    COMPONENT ${RUNIR_INSTALL_COMPONENT}
)

export(EXPORT runircoreTargets
    NAMESPACE runir::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/runircoreTargets.cmake"
)
