set(APP_SOURCES
    main.cpp
)

qt6_add_resources(APP_RES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/res.qrc
)

add_subdirectory(source/model)

add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_RES}
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    model
    ws2_32
    iphlpapi
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nlohmann
)
