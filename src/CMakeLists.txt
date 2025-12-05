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
    ${CMAKE_CURRENT_LIST_DIR}/res/logo/logo.rc
)

if (WIN32 AND MINGW)
    # MinGW 使用不同的链接器参数语法
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        # Release 模式：隐藏控制台窗口
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-Wl,-subsystem,windows"
        )
    else()
        # Debug 模式：显示控制台窗口
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-Wl,-subsystem,console"
        )
    endif()
endif()

target_link_libraries(${PROJECT_NAME}
    driver
    control
    model
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::QuickControls2
    ws2_32
    iphlpapi
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)