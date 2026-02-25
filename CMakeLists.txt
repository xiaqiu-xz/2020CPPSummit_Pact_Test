# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(PactCppDemo)
set(CMAKE_CXX_STANDARD 17)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
# 消费者测试
add_executable(consumer_test
    consumer_test/consumer_test.cpp
)
target_link_libraries(consumer_test
    ${CONAN_LIBS}
    pact_ffi          # Pact Rust FFI 核心库
    pact_cpp_consumer # C++ DSL 封装
)
# 提供者验证测试
add_executable(provider_test
    provider_test/provider_test.cpp
)
target_link_libraries(provider_test ${CONAN_LIBS})