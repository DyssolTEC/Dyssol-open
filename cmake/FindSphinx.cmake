# Look for an executable called sphinx-build
FIND_PROGRAM(SPHINX_EXECUTABLE
             NAMES sphinx-build
             DOC "Path to sphinx-build executable")

INCLUDE(FindPackageHandleStandardArgs)

# Handle standard arguments to find_package like REQUIRED and QUIET
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sphinx
                                  "Failed to find sphinx-build executable"
                                  SPHINX_EXECUTABLE)