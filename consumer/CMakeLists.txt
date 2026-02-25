cmake_minimum_required(VERSION 3.16)
project(pact-cpp-consumer VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 在根级统一 find_package，子目录继承
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(cpprestsdk REQUIRED)

add_subdirectory(src)

install(TARGETS pact-cpp-consumer DESTINATION lib)
install(FILES include/consumer.h include/matchers.h DESTINATION include)

enable_testing()
add_subdirectory(test)