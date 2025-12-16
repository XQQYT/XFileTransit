set(CONTROL_SOURCES
    NetworkController.cpp
    MsgParser/JsonParser.cpp
    FileSyncEngine/FileSyncEngine.cpp
    SettingsController.cpp
)

add_library(control STATIC
    ${CONTROL_SOURCES}
)

target_include_directories(control PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(control driver)
