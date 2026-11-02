cmake_minimum_required(VERSION 3.28)
project(configuration_handler VERSION 1.0.0 LANGUAGES CXX)

add_library(configuration_handler INTERFACE)
add_library(json::configuration_handler ALIAS configuration_handler)

target_include_directories(configuration_handler INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(configuration_handler
    INTERFACE
        decree::decree
)

option(CONFIGURATION_HANDLER_BUILD_TESTS "Build unit tests" OFF)

if(CONFIGURATION_HANDLER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
