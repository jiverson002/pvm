include(FindPkgConfig)

# Probe for Check information.
pkg_check_modules(LIBCHECK REQUIRED check)

# Add include directories for libpvm.
include_directories(${CMAKE_BINARY_DIR}/src)
# Add compile options necessary for Check.
add_compile_options("${LIBCHECK_CFLAGS}")
# Link against libpvm and libcheck.
link_libraries(pvm "${LIBCHECK_LDFLAGS}")

# Create Pep/9 test.
add_executable(test_pep9 test_pep9.c)
# Add it to suite.
add_test(Pep/9 test_pep9)
