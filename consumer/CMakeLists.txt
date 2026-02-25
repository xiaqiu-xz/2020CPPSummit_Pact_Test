cmake_minimum_required(VERSION 3.16)

# 项目名称和版本
project(pact-cpp-consumer VERSION 0.1.1 LANGUAGES CXX)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找 Conan 生成的文件（conanbuildinfo.cmake）
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# 子目录
add_subdirectory(src)

# 安装库和头文件
install(TARGETS pact-cpp-consumer DESTINATION lib)
install(FILES include/consumer.h include/matchers.h DESTINATION include)

# 启用测试
enable_testing()
add_subdirectory(test)

# ────────────────────────────────────────────────────────────────
# 下载并准备 libpact_ffi（最新稳定版 0.5.3，2026 年 2 月数据）
# ────────────────────────────────────────────────────────────────
set(PACT_FFI_VERSION "0.5.3")  # 推荐使用最新版（从 crates.io / GitHub releases 确认）
set(PACT_FFI_DIR "${CMAKE_BINARY_DIR}/pact_ffi")
set(PACT_FFI_SO "${PACT_FFI_DIR}/libpact_ffi.so")

file(MAKE_DIRECTORY "${PACT_FFI_DIR}")

# 只在文件不存在时下载和处理
if(NOT EXISTS "${PACT_FFI_SO}")
  # Linux x86_64 的压缩文件是 .so.gz（单文件 gzip）
  set(PACT_FFI_GZ "${PACT_FFI_DIR}/libpact_ffi-linux-x86_64.so.gz")
  set(PACT_FFI_URL "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/libpact_ffi-linux-x86_64.so.gz")

  message(STATUS "Downloading libpact_ffi v${PACT_FFI_VERSION} from ${PACT_FFI_URL}")

  file(DOWNLOAD "${PACT_FFI_URL}" "${PACT_FFI_GZ}"
    STATUS DOWNLOAD_STATUS
    SHOW_PROGRESS
  )
  list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
  list(GET DOWNLOAD_STATUS 1 ERROR_MSG)
  if(NOT STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "下载 libpact_ffi 失败: ${ERROR_MSG}")
  endif()

  # 使用 gunzip 解压 .gz 文件（CMake 自带 gunzip 支持有限，这里用 execute_process 调用系统 gunzip）
  find_program(GUNZIP gunzip)
  if(NOT GUNZIP)
    message(FATAL_ERROR "系统缺少 gunzip 工具，请安装 gzip 包")
  endif()

  execute_process(
    COMMAND ${GUNZIP} -f "${PACT_FFI_GZ}"
    WORKING_DIRECTORY "${PACT_FFI_DIR}"
    RESULT_VARIABLE GUNZIP_RESULT
  )
  if(NOT GUNZIP_RESULT EQUAL 0)
    message(FATAL_ERROR "gunzip 解压失败，返回码: ${GUNZIP_RESULT}")
  endif()

  # gunzip 后文件名为 libpact_ffi-linux-x86_64.so，重命名为标准 libpact_ffi.so
  file(RENAME
    "${PACT_FFI_DIR}/libpact_ffi-linux-x86_64.so"
    "${PACT_FFI_SO}"
  )

  message(STATUS "libpact_ffi 已下载并解压到: ${PACT_FFI_SO}")
endif()

# 下载 pact.h 头文件（可选，如果你的代码直接 include "pact.h"）
set(PACT_H "${PACT_FFI_DIR}/pact.h")
if(NOT EXISTS "${PACT_H}")
  file(DOWNLOAD
    "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/pact.h"
    "${PACT_H}"
    STATUS DOWNLOAD_STATUS
  )
  list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
  if(NOT STATUS_CODE EQUAL 0)
    message(WARNING "下载 pact.h 失败，但非致命，继续...")
  endif()
endif()

# ────────────────────────────────────────────────────────────────
# 创建 imported target：pact_ffi
# ────────────────────────────────────────────────────────────────
add_library(pact_ffi SHARED IMPORTED GLOBAL)
set_target_properties(pact_ffi PROPERTIES
  IMPORTED_LOCATION "${PACT_FFI_SO}"
  INTERFACE_INCLUDE_DIRECTORIES "${PACT_FFI_DIR}"
)

# 确保 rpath 正确（运行时能找到 .so）
set_target_properties(pact_ffi PROPERTIES
  IMPORTED_NO_SONAME TRUE  # 如果是无版本号的 so，可选
)

# ────────────────────────────────────────────────────────────────
# 消费者测试 target
# ────────────────────────────────────────────────────────────────
add_executable(consumer_test
  consumer_test/consumer_test.cpp
)

target_link_libraries(consumer_test
  PRIVATE
    CONAN_PKG::gtest          # Conan 提供的 GTest
    pact_ffi
    CONAN_PKG::nlohmann_json
    CONAN_PKG::cpprestsdk     # 如果需要
)

target_include_directories(consumer_test
  PRIVATE
    ${CMAKE_SOURCE_DIR}/consumer/include
    ${PACT_FFI_DIR}
)

# 设置 rpath，让可执行文件运行时找到 libpact_ffi.so
set_target_properties(consumer_test PROPERTIES
  BUILD_RPATH "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

# ────────────────────────────────────────────────────────────────
# 提供者验证测试 target（如果有）
# ────────────────────────────────────────────────────────────────
add_executable(provider_test
  provider_test/provider_test.cpp
)

target_link_libraries(provider_test
  PRIVATE
    CONAN_PKG::gtest
    pact_ffi
)

target_include_directories(provider_test
  PRIVATE
    ${CMAKE_SOURCE_DIR}/provider/include
    ${PACT_FFI_DIR}
)

set_target_properties(provider_test PROPERTIES
  BUILD_RPATH "${PACT_FFI_DIR}"
  INSTALL_RPATH "${PACT_FFI_DIR}"
)

# 可选：添加测试
add_test(NAME ConsumerTest COMMAND consumer_test)
add_test(NAME ProviderTest COMMAND provider_test)