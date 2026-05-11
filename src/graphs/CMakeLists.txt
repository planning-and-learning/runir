add_library(graphs
    algorithms/nauty.cpp
    algorithms/nauty_impl.cpp
)
add_library(runir::graphs ALIAS graphs)

set_target_properties(graphs PROPERTIES OUTPUT_NAME runir_graphs)

target_compile_features(graphs PUBLIC cxx_std_20)

target_include_directories(graphs
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(graphs
    PUBLIC
        tyr::core
)

find_path(RUNIR_NAUTY_INCLUDE_DIR
    NAMES nauty.h nausparse.h
)
find_library(RUNIR_NAUTY_LIBRARY
    NAMES nauty
)

if(RUNIR_NAUTY_INCLUDE_DIR AND RUNIR_NAUTY_LIBRARY)
    message(STATUS "Found nauty: ${RUNIR_NAUTY_LIBRARY}")
    target_compile_definitions(graphs PRIVATE RUNIR_HAS_NAUTY)
    target_include_directories(graphs PRIVATE "${RUNIR_NAUTY_INCLUDE_DIR}")
    target_link_libraries(graphs PRIVATE "${RUNIR_NAUTY_LIBRARY}")
else()
    message(STATUS "Nauty not found; runir graph nauty backend will throw at runtime.")
endif()

if(RUNIR_BUILD_SHARED)
    if(APPLE)
        set(RUNIR_GRAPHS_LIBRARY_ORIGIN "@loader_path")
    elseif(UNIX)
        set(RUNIR_GRAPHS_LIBRARY_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(graphs PROPERTIES INSTALL_RPATH "${RUNIR_GRAPHS_LIBRARY_ORIGIN}")
endif()

install(TARGETS graphs
    EXPORT runirgraphsTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runir
    COMPONENT runir
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT runirgraphsTargets
    NAMESPACE runir::
    FILE "runirgraphsTargets.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    COMPONENT runir
)

export(EXPORT runirgraphsTargets
    NAMESPACE runir::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/runirgraphsTargets.cmake"
)
