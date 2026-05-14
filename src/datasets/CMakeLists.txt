add_library(datasets
    equivalence_graph.cpp
    object_graph.cpp
    state_graph.cpp
)
add_library(runir::datasets ALIAS datasets)

set_target_properties(datasets PROPERTIES OUTPUT_NAME runir_datasets)

target_compile_features(datasets PUBLIC cxx_std_20)

target_include_directories(datasets
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(datasets
    PUBLIC
        fmt::fmt-header-only
        tyr::core
    PRIVATE
        $<TARGET_FILE:graphs>
)

add_dependencies(datasets graphs)

target_compile_definitions(datasets
    PUBLIC
        FMT_HEADER_ONLY
)

if(RUNIR_BUILD_SHARED)
    if(APPLE)
        set(RUNIR_DATASETS_LIBRARY_ORIGIN "@loader_path")
    elseif(UNIX)
        set(RUNIR_DATASETS_LIBRARY_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(datasets PROPERTIES INSTALL_RPATH "${RUNIR_DATASETS_LIBRARY_ORIGIN}")
endif()

install(TARGETS datasets
    EXPORT runirTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runir
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
