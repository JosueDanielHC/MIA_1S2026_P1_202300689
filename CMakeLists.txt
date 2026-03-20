cmake_minimum_required(VERSION 3.14)
project(ExtreamFS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# cpp-httplib (header-only, vendored en backend/third_party)
set(CPPHTTPLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/backend/third_party)

# Ejecutable del backend
add_executable(extreamfs
  backend/main.cpp
  backend/server/http_server.cpp
  backend/analyzer/lexer.cpp
  backend/analyzer/parser.cpp
  backend/commands/disk/mkdisk.cpp
  backend/commands/disk/rmdisk.cpp
  backend/commands/disk/fdisk.cpp
  backend/commands/mount/mount.cpp
  backend/commands/fs/mkfs.cpp
  backend/commands/fs/mkdir.cpp
  backend/commands/fs/mkfile.cpp
  backend/commands/fs/cat.cpp
  backend/commands/users/login.cpp
  backend/commands/users/logout.cpp
  backend/commands/users/mkgrp.cpp
  backend/commands/users/rmgrp.cpp
  backend/commands/users/mkusr.cpp
  backend/commands/users/rmusr.cpp
  backend/commands/users/chgrp.cpp
  backend/commands/reports/rep.cpp
  backend/manager/disk_manager.cpp
  backend/manager/mount_manager.cpp
  backend/manager/session_manager.cpp
  backend/manager/fs_manager.cpp
  backend/reports/graphviz.cpp
  backend/reports/generator.cpp
  backend/utils/file_manager.cpp
  backend/utils/logger.cpp
  backend/utils/helpers.cpp
  backend/utils/debug_mbr.cpp
  backend/utils/debug_ebr.cpp
)

target_include_directories(extreamfs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/backend
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/analyzer
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/server
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/commands/disk
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/commands/mount
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/commands/fs
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/commands/users
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/commands/reports
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/manager
  ${CMAKE_CURRENT_SOURCE_DIR}/backend/utils
  ${CPPHTTPLIB_DIR}
)

# En Linux enlazar pthread (requerido por cpp-httplib en algunos entornos)
find_package(Threads REQUIRED)
target_link_libraries(extreamfs PRIVATE Threads::Threads)
