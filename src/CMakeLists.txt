set(LIBRARY_SOURCES pep9.c)

# Generate pvm.h.
configure_file("${PVM_CMAKE_PATH}/pvm.h.in" pvm.h)

# Declare a new library target.
add_library(pvm ${LIBRARY_SOURCES})

# Set VERSION property.
# Set SOVERSION to major number of VERSION. So libpvm.so.X will be a symlink
# to libmylib.so.X.Y.Z.
# Declare public API of your library. This API will be installed for third-party
# application.
set_target_properties(
  pvm PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER pvm.h
)

# Pass top directory and build directory in included directories
target_include_directories(pvm PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# Add compile options for all build types.
target_compile_options(pvm PRIVATE ${COMPILER_OPTIONS})

# Export a pkg-config file. This files allows third-party application to easily
# import your library.
configure_file("${PVM_CMAKE_PATH}/pvm.pc.in" pvm.pc @ONLY)

# Declare files to install
install(TARGETS pvm
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES pvm.pc
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pvm/pkgconfig
)

# Create splint file
string(
  REGEX
  REPLACE ";?([^;]+)" "${CMAKE_CURRENT_SOURCE_DIR}/\\1 "
  SPLINT_SOURCES
  "${LIBRARY_SOURCES}"
)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/.splint-sources" ${SPLINT_SOURCES})
