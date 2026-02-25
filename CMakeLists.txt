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
# 真实目录结构（已通过 CI 确认）：
#   consumer/include/consumer.h    ← #include "consumer.h"
#   consumer/include/matchers.h    ← #include "matchers.h"
#   consumer/src/consumer.cpp
#   consumer/src/matchers.cpp
# 没有 .hpp 文件，没有 pact-cpp/ 子目录
FetchContent_Declare(
  pact_cplusplus
  GIT_REPOSITORY https://github.com/pact-foundation/pact-cplusplus.git
  GIT_TAG        pact-cpp-consumer-v0.1.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pact_cplusplus)

# 已确认的路径，直接硬编码
set(PACT_CPP_INCLUDE_DIR "${pact_cplusplus_SOURCE_DIR}/consumer/include")
set(PACT_CPP_SRC_DIR     "${pact_cplusplus_SOURCE_DIR}/consumer/src")

# ── 4. 下载 libpact_ffi ────────────────────────────────────────
# 格式是 .so.gz（单文件 gzip），用 gunzip 解压，不是 tar
set(PACT_FFI_VERSION "0.5.1")
set(PACT_FFI_DIR "${CMAKE_BINARY_DIR}/pact_ffi")
set(PACT_FFI_SO  "${PACT_FFI_DIR}/libpact_ffi.so")

file(MAKE_DIRECTORY ${PACT_FFI_DIR})

if(NOT EXISTS "${PACT_FFI_SO}")
  set(PACT_FFI_GZ "${PACT_FFI_DIR}/libpact_ffi-linux-x86_64.so.gz")
  message(STATUS "Downloading libpact_ffi v${PACT_FFI_VERSION}...")
  file(DOWNLOAD
    "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/libpact_ffi-linux-x86_64.so.gz"
    "${PACT_FFI_GZ}"
    SHOW_PROGRESS STATUS DL_STATUS
  )
  list(GET DL_STATUS 0 DL_CODE)
  if(NOT DL_CODE EQUAL 0)
    list(GET DL_STATUS 1 DL_ERR)
    message(FATAL_ERROR "下载 libpact_ffi 失败: ${DL_ERR}")
  endif()

  execute_process(
    COMMAND gunzip -f "${PACT_FFI_GZ}"
    RESULT_VARIABLE GZ_RET
  )
  if(NOT GZ_RET EQUAL 0)
    message(FATAL_ERROR "gunzip 失败: ${GZ_RET}")
  endif()

  file(RENAME
    "${PACT_FFI_DIR}/libpact_ffi-linux-x86_64.so"
    "${PACT_FFI_SO}"
  )
  message(STATUS "libpact_ffi 就绪: ${PACT_FFI_SO}")
endif()

# 下载 pact.h（pact_ffi 的 C 头文件）
set(PACT_H "${PACT_FFI_DIR}/pact.h")
if(NOT EXISTS "${PACT_H}")
  file(DOWNLOAD
    "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/pact.h"
    "${PACT_H}" SHOW_PROGRESS
  )
endif()

# ── 5. 注册 pact_ffi 动态库 ───────────────────────────────────
add_library(pact_ffi SHARED IMPORTED GLOBAL)
set_target_properties(pact_ffi PROPERTIES
  IMPORTED_LOCATION "${PACT_FFI_SO}"
)

# ── 6. 编译 pact_cpp_consumer 静态库 ──────────────────────────
# consumer/src/ 里的 .cpp 需要一起编译进来
add_library(pact_cpp_consumer STATIC
  ${PACT_CPP_SRC_DIR}/consumer.cpp
  ${PACT_CPP_SRC_DIR}/matchers.cpp
)
target_include_directories(pact_cpp_consumer
  PUBLIC
    ${PACT_CPP_INCLUDE_DIR}   # consumer/include/ → #include "consumer.h"
    ${PACT_FFI_DIR}           # pact.h
)
target_link_libraries(pact_cpp_consumer PUBLIC pact_ffi)

# ── 7. 消费者测试可执行文件 ───────────────────────────────────
add_executable(consumer_test
  consumer_test/consumer_test.cpp
)
target_link_libraries(consumer_test
  PRIVATE
    GTest::gtest_main
    pact_cpp_consumer            # 传递 include + pact_ffi
    nlohmann_json::nlohmann_json
)
set_target_properties(consumer_test PROPERTIES
  BUILD_RPATH   "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

# ── 8. 提供者验证测试 ──────────────────────────────────────────
add_executable(provider_test
  provider_test/provider_test.cpp
)
target_link_libraries(provider_test
  PRIVATE
    GTest::gtest_main
    pact_ffi
)
target_include_directories(provider_test PRIVATE ${PACT_FFI_DIR})
set_target_properties(provider_test PROPERTIES
  BUILD_RPATH   "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

include(GoogleTest)
gtest_discover_tests(consumer_test)
gtest_discover_tests(provider_test)