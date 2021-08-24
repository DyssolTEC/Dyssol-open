#!/bin/bash
# Script compares two files and delvers error code 1 if it differs

ideal="${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/ideal.res"
real="${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/real.res"

if cmp -s "$ideal" "$real"; then
    printf 'The file "%s" is the same as "%s"\n' "$ideal" "$real"
    exit 0
else
    printf 'The file "%s" is different from "%s"\n' "$ideal" "$real"
    exit 1
fi
