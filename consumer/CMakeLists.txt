cmake_minimum_required(VERSION 3.16)
project(pact-cpp-consumer VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(cpprestsdk REQUIRED)

# ════════════════════════════════════════════════════════
# ① 先下载 pact_ffi（必须在 add_subdirectory 之前！）
# ════════════════════════════════════════════════════════
# set(PACT_FFI_VERSION "0.4.22")
# set(PACT_FFI_DIR "${CMAKE_BINARY_DIR}/pact_ffi")
# file(MAKE_DIRECTORY "${PACT_FFI_DIR}")

# if(WIN32)
#     set(PACT_FFI_LIB_FILE "${PACT_FFI_DIR}/pact_ffi.dll")
#     set(PACT_FFI_URL "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/pact_ffi-windows-x86_64.dll.gz")
# elseif(APPLE)
#     set(PACT_FFI_LIB_FILE "${PACT_FFI_DIR}/libpact_ffi.dylib")
#     # 注意：Apple Silicon 用 aarch64，x86_64 Runner 用下面这个
#     set(PACT_FFI_URL "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/libpact_ffi-macos-x86_64.dylib.gz")
# else()
#     set(PACT_FFI_LIB_FILE "${PACT_FFI_DIR}/libpact_ffi.so")
#     set(PACT_FFI_URL "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/libpact_ffi-linux-x86_64.so.gz")
# endif()

# # 下载并解压库文件
# if(NOT EXISTS "${PACT_FFI_LIB_FILE}")
#     set(PACT_FFI_GZ "${PACT_FFI_DIR}/pact_ffi_download.gz")
#     message(STATUS "Downloading libpact_ffi v${PACT_FFI_VERSION}...")
#     file(DOWNLOAD "${PACT_FFI_URL}" "${PACT_FFI_GZ}" SHOW_PROGRESS STATUS DL_STATUS)
#     list(GET DL_STATUS 0 DL_CODE)
#     if(NOT DL_CODE EQUAL 0)
#         message(FATAL_ERROR "下载 pact_ffi 失败: ${DL_STATUS}")
#     endif()

#     # ✅ 跨平台解压：优先用 CMake 内置（cmake -E tar），不依赖系统工具
#     message(STATUS "Extracting pact_ffi...")
#     execute_process(
#         COMMAND ${CMAKE_COMMAND} -E tar xzf "${PACT_FFI_GZ}"
#         WORKING_DIRECTORY "${PACT_FFI_DIR}"
#         RESULT_VARIABLE GZ_RESULT
#     )

#     if(NOT GZ_RESULT EQUAL 0)
#         # cmake -E tar 对纯 .gz（非 .tar.gz）可能失败，回退到平台工具
#         if(WIN32)
#             # Windows：用 PowerShell 解压
#             execute_process(
#                 COMMAND powershell -Command
#                     "& { Add-Type -AssemblyName System.IO.Compression.FileSystem; $fs = [System.IO.File]::OpenRead('${PACT_FFI_GZ}'); $out = [System.IO.File]::Create('${PACT_FFI_LIB_FILE}'); $gz = [System.IO.Compression.GZipStream]::new($fs, [System.IO.Compression.CompressionMode]::Decompress); $gz.CopyTo($out); $gz.Close(); $out.Close(); $fs.Close() }"
#                 RESULT_VARIABLE PS_RESULT
#             )
#             if(NOT PS_RESULT EQUAL 0)
#                 message(FATAL_ERROR "Windows 解压 pact_ffi 失败")
#             endif()
#         else()
#             # Linux/macOS 回退
#             find_program(GUNZIP gunzip REQUIRED)
#             execute_process(
#                 COMMAND ${GUNZIP} -f "${PACT_FFI_GZ}"
#                 RESULT_VARIABLE GZ_RESULT2
#             )
#             if(NOT GZ_RESULT2 EQUAL 0)
#                 message(FATAL_ERROR "解压 pact_ffi 失败")
#             endif()
#             # 解压后重命名
#             file(GLOB PACT_FFI_EXTRACTED
#                 "${PACT_FFI_DIR}/libpact_ffi-*"
#                 "${PACT_FFI_DIR}/pact_ffi-*"
#             )
#             foreach(f ${PACT_FFI_EXTRACTED})
#                 file(RENAME "${f}" "${PACT_FFI_LIB_FILE}")
#             endforeach()
#         endif()
#     else()
#         # cmake -E tar 成功时重命名
#         file(GLOB PACT_FFI_EXTRACTED
#             "${PACT_FFI_DIR}/libpact_ffi-*"
#             "${PACT_FFI_DIR}/pact_ffi-*"
#         )
#         foreach(f ${PACT_FFI_EXTRACTED})
#             file(RENAME "${f}" "${PACT_FFI_LIB_FILE}")
#         endforeach()
#     endif()

#     message(STATUS "libpact_ffi 就绪: ${PACT_FFI_LIB_FILE}")
# endif()
# # 下载 pact.h 头文件
# set(PACT_H "${PACT_FFI_DIR}/pact.h")
# if(NOT EXISTS "${PACT_H}")
#     message(STATUS "Downloading pact.h...")
#     file(DOWNLOAD
#         "https://github.com/pact-foundation/pact-reference/releases/download/libpact_ffi-v${PACT_FFI_VERSION}/pact.h"
#         "${PACT_H}"
#         STATUS DL_STATUS
#     )
#     list(GET DL_STATUS 0 DL_CODE)
#     if(NOT DL_CODE EQUAL 0)
#         message(FATAL_ERROR "下载 pact.h 失败: ${DL_STATUS}")
#     endif()
# endif()


# ════════════════════════════════════════════════════════
# pact_ffi — 由 CI workflow 提前下载，这里只创建 imported target
# ════════════════════════════════════════════════════════
set(PACT_FFI_DIR "${CMAKE_BINARY_DIR}/pact_ffi")

if(WIN32)
    set(PACT_FFI_LIB_FILE "${PACT_FFI_DIR}/pact_ffi.dll")
elseif(APPLE)
    set(PACT_FFI_LIB_FILE "${PACT_FFI_DIR}/libpact_ffi.dylib")
else()
    set(PACT_FFI_LIB_FILE "${PACT_FFI_DIR}/libpact_ffi.so")
endif()


# 创建 imported target
add_library(pact_ffi SHARED IMPORTED GLOBAL)
set_target_properties(pact_ffi PROPERTIES
    IMPORTED_LOCATION             "${PACT_FFI_LIB_FILE}"
    INTERFACE_INCLUDE_DIRECTORIES "${PACT_FFI_DIR}"
)

# ════════════════════════════════════════════════════════
# ② 再编译子目录（此时 pact_ffi target 和 pact.h 已就绪）
# ════════════════════════════════════════════════════════
add_subdirectory(src)

install(TARGETS pact-cpp-consumer DESTINATION lib)
install(FILES include/consumer.h include/matchers.h DESTINATION include)

enable_testing()
add_subdirectory(test)