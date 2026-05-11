function(configure_pypddl)
    if(RUNIR_DISABLE_PYPDDL_DISCOVERY)
        return()
    endif()

    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(NOT Python3_Interpreter_FOUND)
        return()
    endif()

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import pypddl; print(pypddl.native_prefix())"
        RESULT_VARIABLE pypddl_result
        OUTPUT_VARIABLE pypddl_prefix
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(pypddl_result EQUAL 0 AND EXISTS "${pypddl_prefix}")
        file(GLOB pypddl_cmake_dirs LIST_DIRECTORIES true "${pypddl_prefix}/lib*/cmake")
        foreach(pypddl_cmake_dir IN LISTS pypddl_cmake_dirs)
            get_filename_component(pypddl_lib_dir "${pypddl_cmake_dir}" DIRECTORY)
            list(PREPEND CMAKE_PREFIX_PATH "${pypddl_cmake_dir}" "${pypddl_lib_dir}")
        endforeach()
        list(PREPEND CMAKE_PREFIX_PATH "${pypddl_prefix}")
        set(PYPDDL_NATIVE_PREFIX "${pypddl_prefix}" PARENT_SCOPE)
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
        message(STATUS "Found pypddl native prefix: ${pypddl_prefix}")
    endif()
endfunction()
