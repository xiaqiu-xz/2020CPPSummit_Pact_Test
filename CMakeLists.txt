cmake_minimum_required(VERSION 3.16)
project(PactCppDemo CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# ── 1. GoogleTest ──────────────────────────────────────────────
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(googletest)

# ── 2. nlohmann/json ───────────────────────────────────────────
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ── 3. pact-cplusplus ─────────────────────────────────────────
FetchContent_Declare(
  pact_cplusplus
  GIT_REPOSITORY https://github.com/pact-foundation/pact-cplusplus.git
  GIT_TAG        pact-cpp-consumer-v0.1.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pact_cplusplus)

# ── 自动探测头文件真实路径 ─────────────────────────────────────
# 先找 consumer.hpp 在哪里，无论仓库结构怎么变都能找到
file(GLOB_RECURSE CONSUMER_HPP
  "${pact_cplusplus_SOURCE_DIR}/consumer.hpp"
)
message(STATUS "Found consumer.hpp candidates: ${CONSUMER_HPP}")

# 优先检查已知的几个可能路径
foreach(CANDIDATE
    "${pact_cplusplus_SOURCE_DIR}/consumer/include"
    "${pact_cplusplus_SOURCE_DIR}/consumer/src"
    "${pact_cplusplus_SOURCE_DIR}/include"
    "${pact_cplusplus_SOURCE_DIR}/src"
    "${pact_cplusplus_SOURCE_DIR}"
)
  # 检查是否存在 pact-cpp/consumer.hpp（#include <pact-cpp/consumer.hpp>）
  if(EXISTS "${CANDIDATE}/pact-cpp/consumer.hpp")
    set(PACT_CPP_INCLUDE_DIR "${CANDIDATE}")
    message(STATUS "Found pact-cpp include dir: ${PACT_CPP_INCLUDE_DIR}")
    break()
  endif()
  # 也检查直接 consumer.hpp（#include <consumer.hpp>）
  if(EXISTS "${CANDIDATE}/consumer.hpp")
    set(PACT_CPP_INCLUDE_DIR "${CANDIDATE}")
    message(STATUS "Found pact-cpp include dir (flat): ${PACT_CPP_INCLUDE_DIR}")
    break()
  endif()
endforeach()

if(NOT DEFINED PACT_CPP_INCLUDE_DIR)
  message(FATAL_ERROR
    "找不到 pact-cpp/consumer.hpp！\n"
    "请检查 build/_deps/pact_cplusplus-src/ 的目录结构，\n"
    "并在 CMakeLists.txt 中手动指定 PACT_CPP_INCLUDE_DIR"
  )
endif()

# ── 4. 下载 libpact_ffi ────────────────────────────────────────
set(PACT_FFI_VERSION "0.5.1")
set(PACT_FFI_DIR "${CMAKE_BINARY_DIR}/pact_ffi")
set(PACT_FFI_SO  "${PACT_FFI_DIR}/libpact_ffi.so")

file(MAKE_DIRECTORY ${PACT_FFI_DIR})

if(NOT EXISTS "${PACT_FFI_SO}")
  set(PACT_FFI_GZ  "${PACT_FFI_DIR}/libpact_ffi-linux-x86_64.so.gz")
  set(PACT_FFI_URL
    "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/libpact_ffi-linux-x86_64.so.gz"
  )

  message(STATUS "Downloading libpact_ffi v${PACT_FFI_VERSION}...")
  file(DOWNLOAD "${PACT_FFI_URL}" "${PACT_FFI_GZ}"
    SHOW_PROGRESS STATUS DOWNLOAD_STATUS
  )
  list(GET DOWNLOAD_STATUS 0 CODE)
  if(NOT CODE EQUAL 0)
    list(GET DOWNLOAD_STATUS 1 ERR)
    message(FATAL_ERROR "下载失败: ${ERR}")
  endif()

  execute_process(
    COMMAND gunzip -f "${PACT_FFI_GZ}"
    RESULT_VARIABLE GZ_RESULT
  )
  if(NOT GZ_RESULT EQUAL 0)
    message(FATAL_ERROR "gunzip 失败: ${GZ_RESULT}")
  endif()

  file(RENAME
    "${PACT_FFI_DIR}/libpact_ffi-linux-x86_64.so"
    "${PACT_FFI_SO}"
  )
  message(STATUS "libpact_ffi 就绪: ${PACT_FFI_SO}")
endif()

# 下载头文件 pact.h
set(PACT_H "${PACT_FFI_DIR}/pact.h")
if(NOT EXISTS "${PACT_H}")
  file(DOWNLOAD
    "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/pact.h"
    "${PACT_H}" SHOW_PROGRESS
  )
endif()

# ── 5. 注册 pact_ffi 导入库 ───────────────────────────────────
add_library(pact_ffi SHARED IMPORTED GLOBAL)
set_target_properties(pact_ffi PROPERTIES
  IMPORTED_LOCATION             "${PACT_FFI_SO}"
  INTERFACE_INCLUDE_DIRECTORIES "${PACT_FFI_DIR}"
  # 不在这里设置 pact-cplusplus 的 include，
  # 改在每个 target 里单独设置，更精确
)

# ── 6. 消费者测试 ──────────────────────────────────────────────
add_executable(consumer_test
  consumer_test/consumer_test.cpp
)
target_link_libraries(consumer_test
  PRIVATE
    GTest::gtest_main
    pact_ffi
    nlohmann_json::nlohmann_json
)
# 使用自动探测到的正确路径
target_include_directories(consumer_test PRIVATE
  ${PACT_CPP_INCLUDE_DIR}   # 包含 pact-cpp/consumer.hpp 的目录
  ${PACT_FFI_DIR}           # 包含 pact.h
)
set_target_properties(consumer_test PROPERTIES
  BUILD_RPATH   "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

# ── 7. 提供者验证测试 ──────────────────────────────────────────
# 探测 provider 头文件路径
foreach(CANDIDATE
    "${pact_cplusplus_SOURCE_DIR}/provider/include"
    "${pact_cplusplus_SOURCE_DIR}/provider/src"
    "${pact_cplusplus_SOURCE_DIR}/provider"
)
  if(EXISTS "${CANDIDATE}")
    set(PACT_CPP_PROVIDER_INCLUDE_DIR "${CANDIDATE}")
    break()
  endif()
endforeach()

add_executable(provider_test
  provider_test/provider_test.cpp
)
target_link_libraries(provider_test
  PRIVATE
    GTest::gtest_main
    pact_ffi
)
target_include_directories(provider_test PRIVATE
  ${PACT_CPP_PROVIDER_INCLUDE_DIR}
  ${PACT_FFI_DIR}
)
set_target_properties(provider_test PROPERTIES
  BUILD_RPATH   "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

include(GoogleTest)
gtest_discover_tests(consumer_test)
gtest_discover_tests(provider_test)