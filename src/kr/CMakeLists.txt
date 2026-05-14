add_library(kr
    dl/cnf_grammar/generate.cpp
    dl/cnf_grammar/translate.cpp
    dl/grammar/grammar_factory.cpp
    dl/grammar/parser.cpp
    dl/grammar/parser/parser.cpp
    dl/grammar/parser/parser_instantiations.cpp
    gp/dl/parser.cpp
    gp/dl/parser/parser.cpp
    gp/dl/parser/parser_instantiations.cpp
    gp/dl/policy_factory.cpp
    gp/policy_executor.cpp
)
add_library(runir::kr ALIAS kr)

set_target_properties(kr PROPERTIES OUTPUT_NAME runir_kr)

target_compile_features(kr PUBLIC cxx_std_20)

target_include_directories(kr
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(kr
    PUBLIC
        fmt::fmt-header-only
        tyr::core
)

target_compile_definitions(kr
    PUBLIC
        FMT_HEADER_ONLY
)

if(RUNIR_BUILD_SHARED)
    if(APPLE)
        set(RUNIR_KR_LIBRARY_ORIGIN "@loader_path")
    elseif(UNIX)
        set(RUNIR_KR_LIBRARY_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(kr PROPERTIES INSTALL_RPATH "${RUNIR_KR_LIBRARY_ORIGIN}")
endif()

install(TARGETS kr
    EXPORT runirTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runir
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
