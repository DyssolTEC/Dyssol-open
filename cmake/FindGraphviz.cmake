# - Find Graphviz library
# Find the native Graphviz includes and library
# This module defines
#  Graphviz_INCLUDE_DIR, where to find tiff.h, etc.
#  Graphviz_LIBRARIES, libraries to link against to use Graphviz.
#  Graphviz_FOUND, If false, do not try to use Graphviz.


FIND_PATH(Graphviz_INCLUDE_DIR gvc.h /usr/include/graphviz)

FIND_LIBRARY(Graphviz_LIBRARY_GVC NAMES gvc)
FIND_LIBRARY(Graphviz_LIBRARY_CGRAPH NAMES cgraph)
SET(Graphviz_LIBRARIES ${Graphviz_LIBRARY_GVC} ${Graphviz_LIBRARY_CGRAPH})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Graphviz  DEFAULT_MSG  Graphviz_LIBRARIES  Graphviz_INCLUDE_DIR)

MARK_AS_ADVANCED(Graphviz_INCLUDE_DIR Graphviz_LIBRARIES)
