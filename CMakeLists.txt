cmake_minimum_required(VERSION 3.9)

# Declare project.
project(
  pvm
  VERSION 0.0.1
  LANGUAGES C
  DESCRIPTION "A Pep/X virtual machine."
)

# Toggle static or shared library.
option(
  BUILD_SHARED_LIBS
  "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)"
  OFF
)

# Set default compiler options.
set(
  COMPILER_OPTIONS
  -ansi -pedantic -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith -Wcast-align
  -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls
  -Wnested-externs -Winline -Wno-long-long -Wuninitialized -Wstrict-prototypes
  -Wno-sign-conversion -Wno-unused-function
  CACHE STRING "Additional options to pass to compiler."
)

# Set module path for CMake.
set(PVM_CMAKE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PVM_CMAKE_PATH}")

# Create install rules.
include(GNUInstallDirs)

# Generate .splintrc
configure_file("${CMAKE_SOURCE_DIR}/cmake/splintrc.in" .splintrc)

# Build library.
add_subdirectory(src)

# Set up testing.
enable_testing()

# Build tests.
add_subdirectory(t)

