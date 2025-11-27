set(CONTROL_SOURCES
    NetworkController.cpp
    MsgParser/JsonParser.cpp
    FileSyncEngine/FileSyncEngine.cpp
)

set(CONTROL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/control/NetworkController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/control/MsgParser/Parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/control/MsgParser/JsonParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/control/FileSyncEngine/FileSyncEngine.h
)

add_library(control STATIC
    ${CONTROL_SOURCES}
    ${CONTROL_HEADERS}
)

target_include_directories(control PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(control driver)
