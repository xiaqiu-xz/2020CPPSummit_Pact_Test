cmake_minimum_required(VERSION 3.16)
project(pact-consumer-tests)

# Conan v2: 不再需要 conanbuildinfo.cmake
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(cpprestsdk REQUIRED)

add_executable(${PROJECT_NAME}
    src/consumer_test.cpp
    src/todo.cpp
    src/matchers_tests.cpp
    src/multipart_parser.cpp
)

target_include_directories(pact-consumer-tests
    PRIVATE ../include
)

# 输出到 test/bin/，对应 CI 里的运行路径
set_target_properties(pact-consumer-tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/bin"
)

if(WIN32)
    target_link_libraries(pact-consumer-tests
        PRIVATE
            GTest::gtest GTest::gtest_main
            nlohmann_json::nlohmann_json
            cpprestsdk::cpprest
            pact-cpp-consumer
            ws2_32 userenv crypt32 secur32 dnsapi ncrypt ntdll
    )
elseif(UNIX)
    target_link_libraries(pact-consumer-tests
        PRIVATE
            GTest::gtest GTest::gtest_main
            nlohmann_json::nlohmann_json
            cpprestsdk::cpprest
            pact-cpp-consumer
    )
endif()

add_test(NAME consumer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin
    COMMAND ${PROJECT_NAME}
)