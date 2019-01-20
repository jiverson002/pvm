cmake_minimum_required(VERSION 3.12)

project(pvm VERSION 0.0.1 LANGUAGES C DESCRIPTION "A Pep/X virtual machine.")

set(COMPILER_OPTIONS -ansi -pedantic -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wuninitialized -Wstrict-prototypes -Wno-sign-conversion -Wno-unused-function CACHE STRING "Additional options to pass to compiler.")

set(LIBRARY_SOURCES src/pep9.c)

option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)

# Declare a new library target.
add_library(pvm ${LIBRARY_SOURCES})

# Set VERSION property.
# Set SOVERSION to major number of VERSION. So libpvm.so.X will be a symlink
# to libmylib.so.X.Y.Z.
# Declare public API of your library. This API will be installed for third-party
# application.
set_target_properties(pvm PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER pvm.h
)

# Pass top directory and build directory in included directories
target_include_directories(pvm PRIVATE . ${CMAKE_BINARY_DIR})

# Add compile options for all build types.
target_compile_options(pvm PRIVATE ${COMPILER_OPTIONS})

# Create install rule for your library.
include(GNUInstallDirs)

# Declare files to install
install(TARGETS pvm
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export a pkg-config file. This files allows third-party application to easily
# import your library.
configure_file(pvm.pc.in pvm.pc @ONLY)

# Install generated file.
install(FILES ${CMAKE_BINARY_DIR}/pvm.pc
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

# Generate pvm.h.
configure_file(include/pvm.h.in pvm.h)

# Generate .splintrc
list(TRANSFORM LIBRARY_SOURCES PREPEND ${CMAKE_SOURCE_DIR}/)
list(JOIN LIBRARY_SOURCES " " SPLINT_SOURCES)
configure_file(.splintrc.in .splintrc)
configure_file(.splint-sources.in .splint-sources)
