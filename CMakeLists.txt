cmake_minimum_required(VERSION 3.10)

# Project name, version and language
project(
  bin2shellcode
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Werror -Wpedantic)

add_library(bin2shellcode_lib STATIC src/converter/converter.cpp)
set_target_properties(bin2shellcode_lib PROPERTIES OUTPUT_NAME "bin2shellcode")

target_include_directories(
  bin2shellcode_lib
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE bin2shellcode_lib)
target_compile_definitions(${PROJECT_NAME}
                           PRIVATE B2S_VERSION="${PROJECT_VERSION}")

# Installation
install(
  TARGETS ${PROJECT_NAME} bin2shellcode_lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)

install(DIRECTORY include/ DESTINATION include)

install(FILES README.md LICENSE DESTINATION share/${PROJECT_NAME})

# Cross-platform packaging with CPack
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# .deb Package stuff
set(CPACK_PACKAGE_CONTACT "Erick Sánchez Vera <Erickdeveloper2000@outlook.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS " ")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

# Generator: ZIP on Windows, TGZ on other systems
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "STGZ;TGZ;TZ;DEB")
endif()
# Final package name: bin2shellcode-x.x.x-Linux/TGZ or .zip
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

include(CPack)

# Target for uninstall
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(
  uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  COMMENT "Uninstalling ${PROJECT_NAME}")

# Target to completely clean the build directory
add_custom_target(
  clean-build
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
  COMMENT "Removing build directory")

find_program(CLANG_FORMAT clang-format)
file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*.[ch]pp"
     "${PROJECT_SOURCE_DIR}/*.[ch]xx")

add_custom_target(
  clang-format
  COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SRC}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running clang-format on source files")

find_program(CMAKE_FORMAT_BIN NAMES cmake-format REQUIRED)

file(
  GLOB_RECURSE
  CMAKE_FORMAT_FILES
  CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/CMakeLists.txt"
  "${PROJECT_SOURCE_DIR}/*/CMakeLists.txt"
  "${PROJECT_SOURCE_DIR}/*.cmake"
  "${PROJECT_SOURCE_DIR}/*.cmake.in")

add_custom_target(
  cmake-format
  COMMAND ${CMAKE_FORMAT_BIN} -i ${CMAKE_FORMAT_FILES}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Formateando scripts CMake con cmake-format"
  VERBATIM)

# Automated tests
enable_testing()
add_executable(test_converter tests/test_converter.cpp)
target_link_libraries(test_converter PRIVATE bin2shellcode_lib)
target_compile_definitions(
  test_converter PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests")

# Name of each test format
set(FORMAT_NAMES
    Raw
    Base64
    C
    Ruby
    Python
    PowerShell
    CSharp
    Hex
    UUID
    Go
    Rust)

foreach(idx RANGE 0 10)
  math(EXPR arg "${idx}+1")
  list(GET FORMAT_NAMES ${idx} fmtName)
  add_test(NAME ${fmtName} COMMAND test_converter ${arg})
endforeach()
