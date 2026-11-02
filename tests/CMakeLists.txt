include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Threads REQUIRED)

configure_file(
    resources/test_configuration.json
    ${CMAKE_CURRENT_BINARY_DIR}/resources/test_configuration.json
    COPYONLY
)

add_executable(json_configuration_handler_test
    json_configuration_handler.cpp
)

target_compile_definitions(json_configuration_handler_test
    PRIVATE
        TEST_RESOURCES_DIR="${CMAKE_CURRENT_BINARY_DIR}/resources"
)

target_link_libraries(json_configuration_handler_test
    PRIVATE
        json::configuration_handler
        GTest::gtest_main
        Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(json_configuration_handler_test)
