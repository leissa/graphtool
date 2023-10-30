add_executable(graphtool main.cpp)

target_sources(graphtool PRIVATE main.cpp)
target_link_libraries(graphtool PRIVATE fe)
target_include_directories(graphtool
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(graphtool PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
if (MSVC AND BUILD_SHARED_LIBS AND FE_ABSL)
    target_compile_definitions(libgraphtool PUBLIC ABSL_CONSUME_DLL)
endif()

add_subdirectory(graphtool)
