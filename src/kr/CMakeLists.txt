add_library(kr SHARED
    dl/cnf_grammar/generate.cpp
    dl/cnf_grammar/translate.cpp
    dl/grammar/base/grammar_factory.cpp
    dl/grammar/base/parser.cpp
    dl/grammar/parser/base/grammar_parser.cpp
    dl/grammar/parser/base/grammar_parser_instantiations.cpp
    dl/grammar/parser/base/parser.cpp
    dl/grammar/parser/base/parser_instantiations.cpp
    dl/grammar/parser/ext/parser.cpp
    dl/grammar/parser/ext/parser_instantiations.cpp
    dl/grammar/parser/uns/grammar_parser.cpp
    dl/grammar/parser/uns/grammar_parser_instantiations.cpp
    ps/base/dl/parser.cpp
    ps/base/dl/parser/parser.cpp
    ps/base/dl/parser/parser_instantiations.cpp
    ps/base/dl/sketch_factory.cpp
    ps/base/sketch_executor.cpp
    ps/ext/dl/module_factory.cpp
    ps/ext/dl/parser/parser.cpp
    ps/ext/dl/parser.cpp
    ps/ext/module_program_executor.cpp
    uns/dl/parser.cpp
    uns/dl/parser/parser.cpp
    uns/dl/parser/parser_instantiations.cpp
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
        yggdrasil::yggdrasil
        fmt::fmt-header-only
        tyr::core
)

target_compile_definitions(kr
    PUBLIC
        FMT_HEADER_ONLY
)

if(APPLE)
    set(RUNIR_KR_LIBRARY_ORIGIN "@loader_path")
elseif(UNIX)
    set(RUNIR_KR_LIBRARY_ORIGIN "$ORIGIN")
endif()

set_target_properties(kr PROPERTIES INSTALL_RPATH "${RUNIR_KR_LIBRARY_ORIGIN}")

install(TARGETS kr
    EXPORT runirTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runir
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
