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
# 注意：默认分支是 master，不是 main
FetchContent_Declare(
  pact_cplusplus
  GIT_REPOSITORY https://github.com/pact-foundation/pact-cplusplus.git
  GIT_TAG        pact-cpp-consumer-v0.1.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pact_cplusplus)

# ── 4. 下载 libpact_ffi ────────────────────────────────────────
# 关键修复：release 格式是 .so.gz（单文件 gzip），不是 tar.gz！
# 用 gunzip 解压，不是 tar -xzf
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
    SHOW_PROGRESS
    STATUS DOWNLOAD_STATUS
  )
  list(GET DOWNLOAD_STATUS 0 DOWNLOAD_CODE)
  if(NOT DOWNLOAD_CODE EQUAL 0)
    list(GET DOWNLOAD_STATUS 1 DOWNLOAD_ERROR)
    message(FATAL_ERROR "下载失败: ${DOWNLOAD_ERROR}")
  endif()

  # .so.gz 用 gunzip 解压（不是 tar）
  execute_process(
    COMMAND gunzip -f "${PACT_FFI_GZ}"
    RESULT_VARIABLE GZ_RESULT
  )
  if(NOT GZ_RESULT EQUAL 0)
    message(FATAL_ERROR "gunzip 失败，返回码: ${GZ_RESULT}")
  endif()

  # gunzip 后文件名带平台后缀，重命名为标准名
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
    "${PACT_H}"
    SHOW_PROGRESS
  )
endif()

# ── 5. 注册 pact_ffi 导入库 ───────────────────────────────────
add_library(pact_ffi SHARED IMPORTED GLOBAL)
set_target_properties(pact_ffi PROPERTIES
  IMPORTED_LOCATION             "${PACT_FFI_SO}"
  INTERFACE_INCLUDE_DIRECTORIES "${PACT_FFI_DIR};${pact_cplusplus_SOURCE_DIR}/consumer/include"
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
target_include_directories(consumer_test PRIVATE
  ${pact_cplusplus_SOURCE_DIR}/consumer/include
  ${PACT_FFI_DIR}
)
set_target_properties(consumer_test PROPERTIES
  BUILD_RPATH   "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

# ── 7. 提供者验证测试 ──────────────────────────────────────────
add_executable(provider_test
  provider_test/provider_test.cpp
)
target_link_libraries(provider_test
  PRIVATE
    GTest::gtest_main
    pact_ffi
)
target_include_directories(provider_test PRIVATE
  ${pact_cplusplus_SOURCE_DIR}/provider/include
  ${PACT_FFI_DIR}
)
set_target_properties(provider_test PROPERTIES
  BUILD_RPATH   "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

include(GoogleTest)
gtest_discover_tests(consumer_test)
gtest_discover_tests(provider_test)