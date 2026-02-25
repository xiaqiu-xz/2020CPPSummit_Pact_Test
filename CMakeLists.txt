cmake_minimum_required(VERSION 3.16)
project(PactCppDemo CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# ── 1. GoogleTest ──────────────────────────────────────────
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# ── 2. nlohmann/json ──────────────────────────────────────
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# ── 3. pact-cplusplus (C++ DSL for Pact) ─────────────────
# 官方仓库：https://github.com/pact-foundation/pact-cplusplus
# 修复：默认分支是 master 不是 main，用 release tag 锁定更稳定
FetchContent_Declare(
  pact_cplusplus
  GIT_REPOSITORY https://github.com/pact-foundation/pact-cplusplus.git
  GIT_TAG        pact-cpp-consumer-v0.1.0   # 当前最新 release tag
  GIT_SHALLOW    TRUE                        # 只拉最近一次 commit，加速下载
)
FetchContent_MakeAvailable(pact_cplusplus)

# ── 4. 下载预编译的 pact_ffi 动态库（Rust 核心）──────────
# pact-cplusplus 依赖 libpact_ffi.so，需要单独下载
set(PACT_FFI_VERSION "0.4.25")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(PACT_FFI_ARCHIVE "libpact_ffi-linux-x86_64.tar.gz")
  set(PACT_FFI_LIB    "libpact_ffi.so")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(PACT_FFI_ARCHIVE "libpact_ffi-osx-x86_64.tar.gz")
  set(PACT_FFI_LIB    "libpact_ffi.dylib")
endif()

set(PACT_FFI_URL
  "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/${PACT_FFI_ARCHIVE}"
)
set(PACT_FFI_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/pact_ffi")

# 下载并解压 pact_ffi
file(MAKE_DIRECTORY ${PACT_FFI_DOWNLOAD_DIR})
if(NOT EXISTS "${PACT_FFI_DOWNLOAD_DIR}/${PACT_FFI_LIB}")
  message(STATUS "Downloading pact_ffi v${PACT_FFI_VERSION}...")
  file(DOWNLOAD
    ${PACT_FFI_URL}
    "${PACT_FFI_DOWNLOAD_DIR}/${PACT_FFI_ARCHIVE}"
    SHOW_PROGRESS
  )
  execute_process(
    COMMAND tar -xzf "${PACT_FFI_ARCHIVE}"
    WORKING_DIRECTORY ${PACT_FFI_DOWNLOAD_DIR}
  )
endif()

# 注册 pact_ffi 为导入库
add_library(pact_ffi SHARED IMPORTED)
set_target_properties(pact_ffi PROPERTIES
  IMPORTED_LOCATION "${PACT_FFI_DOWNLOAD_DIR}/${PACT_FFI_LIB}"
  INTERFACE_INCLUDE_DIRECTORIES "${pact_cplusplus_SOURCE_DIR}/consumer/include"
)

# ── 消费者测试可执行文件 ──────────────────────────────────
add_executable(consumer_test
  consumer_test/consumer_test.cpp
)
target_link_libraries(consumer_test
  PRIVATE
    GTest::gtest_main
    pact_ffi
    nlohmann_json::nlohmann_json
)
target_include_directories(consumer_test
  PRIVATE
    ${pact_cplusplus_SOURCE_DIR}/consumer/include
)
# 运行时需要找到 libpact_ffi.so
set_target_properties(consumer_test PROPERTIES
  BUILD_RPATH "${PACT_FFI_DOWNLOAD_DIR}"
)

# ── 提供者验证测试 ────────────────────────────────────────
add_executable(provider_test
  provider_test/provider_test.cpp
)
target_link_libraries(provider_test
  PRIVATE
    GTest::gtest_main
    pact_ffi
)
target_include_directories(provider_test
  PRIVATE
    ${pact_cplusplus_SOURCE_DIR}/provider/include
)
set_target_properties(provider_test PROPERTIES
  BUILD_RPATH "${PACT_FFI_DOWNLOAD_DIR}"
)

# ── CTest 集成 ────────────────────────────────────────────
include(GoogleTest)
gtest_discover_tests(consumer_test)
gtest_discover_tests(provider_test)
