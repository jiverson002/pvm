# Setup environment for Check.
find_package(Check REQUIRED)

# Add check include directories.
include_directories(test_pep9 PRIVATE "${CHECK_INCLUDE_DIRS}")
# Add check library directories.
link_directories("${CHECK_LIBRARY_DIRS}")

# Create Pep/9 test.
add_executable(test_pep9 test_pep9.c)
# Link against libpvm and libcheck.
target_link_libraries(test_pep9 pvm "${CHECK_LIBRARIES}")
# Add it to suite.
add_test(pep9 test_pep9)
