add_subdirectory(graphs)

add_library(core
    cnf_grammar/generate.cpp
    cnf_grammar/translate.cpp
    datasets/state_graph.cpp
    grammar/parser.cpp
    grammar/parser/parser.cpp
    grammar/parser/parser_instantiations.cpp
    runir.cpp
)
add_library(runir::core ALIAS core)

set_target_properties(core PROPERTIES OUTPUT_NAME runir_core)

target_compile_features(core PUBLIC cxx_std_20)

target_include_directories(core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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

if(RUNIR_BUILD_SHARED)
    if(APPLE)
        set(RUNIR_LIBRARY_ORIGIN "@loader_path")
    elseif(UNIX)
        set(RUNIR_LIBRARY_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(core PROPERTIES INSTALL_RPATH "${RUNIR_LIBRARY_ORIGIN}")
endif()

install(TARGETS core
    EXPORT runircoreTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runir
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT runircoreTargets
    NAMESPACE runir::
    FILE "runircoreTargets.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    COMPONENT runir
)

export(EXPORT runircoreTargets
    NAMESPACE runir::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/runircoreTargets.cmake"
)
