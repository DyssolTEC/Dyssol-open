add_executable(DyssolC
    ${CMAKE_SOURCE_DIR}/DyssolConsole/ConfigFileParser.cpp
    ${CMAKE_SOURCE_DIR}/DyssolConsole/main.cpp
)

TARGET_LINK_LIBRARIES(DyssolC Dyssol)
INSTALL(TARGETS DyssolC DESTINATION ${CMAKE_INSTALL_LIBDIR})
