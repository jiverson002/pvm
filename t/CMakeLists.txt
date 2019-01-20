include(FindPkgConfig)

# Probe for Check information.
pkg_check_modules(LIBCHECK REQUIRED check)

# Create Pep/9 test.
add_executable(test_pep9 test_pep9.c)
# Link against libpvm and libcheck.
target_link_libraries(test_pep9 pvm "${LIBCHECK_LDFLAGS}")
# Add compile options necessary for Check.
target_compile_options(test_pep9 PRIVATE ${LIBCHECK_CFLAGS})
# Add it to suite.
add_test(Pep/9 test_pep9)
