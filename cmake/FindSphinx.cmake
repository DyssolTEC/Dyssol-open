# Look for an executable called sphinx-build
FIND_PROGRAM(SPHINX_EXECUTABLE
             NAMES sphinx-build
             DOC "Path to sphinx-build executable")

INCLUDE(FindPackageHandleStandardArgs)

IF (SPHINX_EXECUTABLE)
  # Run the command to get version
  EXECUTE_PROCESS(COMMAND ${SPHINX_EXECUTABLE} --version OUTPUT_VARIABLE SPHINX_VERSION_RAW)  
  # Extract the version from the output 
  STRING(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)" SPHINX_VERSION ${SPHINX_VERSION_RAW})
  # STRING(STRIP ${SPHINX_VERSION_RAW} SPHINX_VERSION)
ENDIF()

# Handle standard arguments to find_package like REQUIRED and QUIET
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sphinx
                                  "Failed to find sphinx-build executable"
                                  SPHINX_EXECUTABLE 
                                  SPHINX_VERSION)