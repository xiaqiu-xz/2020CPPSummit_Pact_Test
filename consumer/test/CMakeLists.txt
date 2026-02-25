cmake_minimum_required(VERSION 3.16)
project(pact-consumer-tests)

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
    PRIVATE
        ../include
        ${PACT_FFI_DIR}   # 继承自根 CMakeLists 的变量
)

set_target_properties(pact-consumer-tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/bin"
    BUILD_RPATH  "${PACT_FFI_DIR}"   # Linux/macOS 运行时找到 .so/.dylib
    INSTALL_RPATH "${PACT_FFI_DIR}"
)

if(WIN32)
    target_link_libraries(pact-consumer-tests PRIVATE
        pact-cpp-consumer
        pact_ffi                          # ← pact ffi
        GTest::gtest GTest::gtest_main
        nlohmann_json::nlohmann_json
        cpprestsdk::cpprest
        ws2_32 userenv crypt32 secur32 dnsapi ncrypt ntdll
    )
elseif(UNIX)
    target_link_libraries(pact-consumer-tests PRIVATE
        pact-cpp-consumer
        pact_ffi                          # ← pact ffi
        GTest::gtest GTest::gtest_main
        nlohmann_json::nlohmann_json
        cpprestsdk::cpprest
    )
endif()

add_test(NAME consumer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin
    COMMAND ${PROJECT_NAME}
)