cmake_minimum_required(VERSION 3.29 FATAL_ERROR)
project(GraphTool VERSION 0.2.0)

include(GNUInstallDirs)

# Make `make test`/`ninja test` build first instead of testing a stale binary.
set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
include(CTest) # defines BUILD_TESTING

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
if(MSVC)
    add_compile_options(/utf-8 /wd4146 /wd4245)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}; shared libs: ${BUILD_SHARED_LIBS}")

if(WIN32)
    add_compile_definitions(NOMINMAX) # prevents windows.h defining min/max macros
else()
    add_compile_options(-Wall -Wextra)
endif()

add_subdirectory(submodules/fe)

add_executable(graphtool)
target_sources(graphtool
    PRIVATE
        src/main.cpp
        src/graphtool/driver.cpp
        src/graphtool/graph.cpp
        src/graphtool/lexer.cpp
        src/graphtool/parser.cpp
        src/graphtool/tok.cpp
        include/graphtool/driver.h
        include/graphtool/graph.h
        include/graphtool/lexer.h
        include/graphtool/parser.h
        include/graphtool/tok.h
)
target_link_libraries(graphtool PRIVATE fe)
# What `--version` reports; both derive from the project(... VERSION) calls.
# fe's project() only sets fe_VERSION within its own directory scope, so fetch it from there.
get_directory_property(fe_version DIRECTORY submodules/fe DEFINITION fe_VERSION)
target_compile_definitions(graphtool PRIVATE GRAPHTOOL_VERSION="${GraphTool_VERSION}" FE_VERSION="${fe_version}")
target_include_directories(graphtool
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(graphtool PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

install(TARGETS graphtool RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/graphtool")

# `test/run.sh` is the whole test suite -- CI calls the very same script. It knows which cases there
# are and which are skipped, so ask it rather than duplicating that here; one `add_test` per case
# makes `ctest -R cytron` and a per-case verdict possible.
if(BUILD_TESTING)
    find_program(BASH_PROGRAM bash)
    if(BASH_PROGRAM)
        set(graphtool_run_sh "${PROJECT_SOURCE_DIR}/test/run.sh")
        # The case list is baked in at configure time, so a new input or an edit to the skip list
        # has to re-run CMake: CONFIGURE_DEPENDS on the inputs, and the script itself as a dep.
        file(GLOB graphtool_inputs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/test/*.dot")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${graphtool_run_sh}")

        execute_process(
            COMMAND "${BASH_PROGRAM}" "${graphtool_run_sh}" --list
            OUTPUT_VARIABLE graphtool_cases
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE graphtool_list_result
        )
        if(NOT graphtool_list_result EQUAL 0)
            message(FATAL_ERROR "`test/run.sh --list` failed with: ${graphtool_list_result}")
        endif()

        string(REPLACE "\n" ";" graphtool_cases "${graphtool_cases}")
        foreach(graphtool_case IN LISTS graphtool_cases)
            add_test(
                NAME ${graphtool_case}
                COMMAND "${BASH_PROGRAM}" "${graphtool_run_sh}" --exe $<TARGET_FILE:graphtool> ${graphtool_case}
            )
        endforeach()
    else()
        message(STATUS "No bash found: `test/run.sh` and hence the tests are unavailable")
    endif()
endif()
