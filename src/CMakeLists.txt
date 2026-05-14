add_subdirectory(graphs)
add_subdirectory(datasets)
add_subdirectory(kr)

add_library(core INTERFACE)
add_library(runir::core ALIAS core)

target_compile_features(core INTERFACE cxx_std_20)

target_include_directories(core
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(core
    INTERFACE
        runir::datasets
        runir::graphs
        runir::kr
)

target_compile_definitions(core
    INTERFACE
        FMT_HEADER_ONLY
)

install(TARGETS core
    EXPORT runirTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT runirTargets
    NAMESPACE runir::
    FILE "runirTargets.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    COMPONENT runir
)

export(EXPORT runirTargets
    NAMESPACE runir::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/runirTargets.cmake"
)
