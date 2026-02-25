cmake_minimum_required(VERSION 3.16)
project(pact-cpp-consumer VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Conan v2: 不再需要 conanbuildinfo.cmake
find_package(nlohmann_json REQUIRED)
find_package(cpprestsdk REQUIRED)

add_library(pact-cpp-consumer consumer.cpp matchers.cpp)

target_include_directories(pact-cpp-consumer
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(pact-cpp-consumer
    PUBLIC
        nlohmann_json::nlohmann_json
        cpprestsdk::cpprest
)