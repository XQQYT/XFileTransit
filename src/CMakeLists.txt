find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

find_package(Qt6 QUIET COMPONENTS Quick)
if(NOT TARGET Qt6::Quick)
    message(WARNING "Qt6 Quick component not found")
endif()

find_package(Qt6 QUIET COMPONENTS QuickControls2)
if(NOT TARGET Qt6::QuickControls2)
    message(WARNING "Qt6 QuickControls2 component not found")
endif()

set(APP_SOURCES
    main.cpp
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/translations)

find_program(LUPDATE_EXECUTABLE 
    NAMES 
        lupdate
    HINTS
        ${Qt6_DIR}/../../../bin
    REQUIRED
)

if(LUPDATE_EXECUTABLE)
    message(STATUS "Found lupdate: ${LUPDATE_EXECUTABLE}")
    
    add_custom_target(update-translations
        COMMAND ${LUPDATE_EXECUTABLE}
            ui
            -recursive
            -extensions qml
            -ts translations/zh_CN.ts
                 translations/en.ts
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Extracting translatable strings..."
    )
endif()

find_program(LRELEASE_EXECUTABLE 
    NAMES 
        lrelease
    HINTS
        ${Qt6_DIR}/../../../bin
    REQUIRED
)
if(LRELEASE_EXECUTABLE)
    add_custom_command(
        OUTPUT 
            res/translations/zh_CN.qm
            res/translations/en.qm
        COMMAND ${CMAKE_COMMAND} -E make_directory res/translations
        COMMAND ${LRELEASE_EXECUTABLE}
            translations/zh_CN.ts -qm res/translations/zh_CN.qm
        COMMAND ${LRELEASE_EXECUTABLE}
            translations/en.ts -qm res/translations/en.qm
        DEPENDS 
            translations/zh_CN.ts
            translations/en.ts
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling translation files"
        VERBATIM
    )
    add_custom_target(compile-translations ALL
    DEPENDS 
        res/translations/zh_CN.qm
        res/translations/en.qm
    )
endif()

set(QML_FILES
    ui/ConnectRequestDialog.qml
    ui/DeviceListWindow.qml
    ui/GeneralDialog.qml
    ui/LoadingDialog.qml
    ui/MainWindow.qml
    ui/NetworkInfoDialog.qml
    ui/SettingsWindow.qml
)

qt_add_translation(QM_FILES
    ${QML_FILES}
    TS_FILES 
        translations/zh_CN.ts
        translations/en.ts
)

qt6_add_resources(APP_RES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/res.qrc
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_OUTPUT)
endif()

add_subdirectory(source/driver)
add_subdirectory(source/control)
add_subdirectory(source/model)

add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_RES}
)
if(LUPDATE_EXECUTABLE)
    add_dependencies(${PROJECT_NAME} update-translations)
endif()

# Windows 特有的资源文件
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/res/logo/logo.rc
    )
endif()

if(WIN32 AND MINGW)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-Wl,-subsystem,windows"
        )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-Wl,-subsystem,console"
        )
    endif()
endif()
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Quick
    Qt6::QuickControls2
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    driver
    control
    model
)

# 平台特有的系统库
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        ws2_32
        iphlpapi
    )
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC
        pthread
        dl
    )
endif()