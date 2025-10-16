set(APP_SOURCES
    main.cpp
)

qt6_add_resources(APP_RES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/res.qrc
)
add_subdirectory(source/driver)
add_subdirectory(source/control)
add_subdirectory(source/model)

add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_RES}
)

target_link_libraries(${PROJECT_NAME}
    driver
    control
    model
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    ws2_32
    iphlpapi
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
