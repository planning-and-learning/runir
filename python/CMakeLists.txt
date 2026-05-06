find_package(Python 3.8 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH})
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

set(PYTEMPLATE_FEATURE_PACKAGE_DIR "pytemplate_feature")
set(PYTEMPLATE_FEATURE_NATIVE_DIR "${PYTEMPLATE_FEATURE_PACKAGE_DIR}/native")
set(PYTEMPLATE_FEATURE_NATIVE_LIBDIR "${PYTEMPLATE_FEATURE_NATIVE_DIR}/${CMAKE_INSTALL_LIBDIR}")

nanobind_add_module(_core
    src/bindings.cpp
)

target_link_libraries(_core PRIVATE template_feature::core)

if(BUILD_SHARED_LIBS)
    if(APPLE)
        set(PYTEMPLATE_FEATURE_LIBRARY_ORIGIN "@loader_path/native/${CMAKE_INSTALL_LIBDIR}")
    elseif(UNIX)
        set(PYTEMPLATE_FEATURE_LIBRARY_ORIGIN "$ORIGIN/native/${CMAKE_INSTALL_LIBDIR}")
    endif()

    if(PYTEMPLATE_FEATURE_LIBRARY_ORIGIN)
        set_target_properties(_core PROPERTIES
            BUILD_RPATH "${PYTEMPLATE_FEATURE_LIBRARY_ORIGIN}"
            INSTALL_RPATH "${PYTEMPLATE_FEATURE_LIBRARY_ORIGIN}")
    endif()
endif()

install(TARGETS _core DESTINATION "${PYTEMPLATE_FEATURE_PACKAGE_DIR}" COMPONENT pytemplate_feature)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pytemplate_feature/__init__.py"
        DESTINATION "${PYTEMPLATE_FEATURE_PACKAGE_DIR}"
        COMPONENT pytemplate_feature)

add_custom_target(pytemplate_feature_wheel DEPENDS _core)

