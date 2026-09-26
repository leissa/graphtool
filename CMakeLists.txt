cmake_minimum_required(VERSION 3.29 FATAL_ERROR)
project(GraphTool VERSION 0.2.0)

include(GNUInstallDirs)

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
