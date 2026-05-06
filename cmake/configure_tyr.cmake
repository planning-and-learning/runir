function(configure_tyr)
    if(TEMPLATE_FEATURE_DISABLE_TYR_DISCOVERY)
        return()
    endif()

    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(NOT Python3_Interpreter_FOUND)
        return()
    endif()

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import pytyr; print(pytyr.native_prefix())"
        RESULT_VARIABLE tyr_result
        OUTPUT_VARIABLE tyr_prefix
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(tyr_result EQUAL 0 AND EXISTS "${tyr_prefix}")
        file(GLOB tyr_cmake_dirs LIST_DIRECTORIES true "${tyr_prefix}/lib*/cmake")
        foreach(tyr_cmake_dir IN LISTS tyr_cmake_dirs)
            get_filename_component(tyr_lib_dir "${tyr_cmake_dir}" DIRECTORY)
            list(PREPEND CMAKE_PREFIX_PATH "${tyr_cmake_dir}" "${tyr_lib_dir}")
        endforeach()
        list(PREPEND CMAKE_PREFIX_PATH "${tyr_prefix}")
        set(TYR_NATIVE_PREFIX "${tyr_prefix}" PARENT_SCOPE)
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
        message(STATUS "Found tyr native prefix: ${tyr_prefix}")
    endif()
endfunction()
