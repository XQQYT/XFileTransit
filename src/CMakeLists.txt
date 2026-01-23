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
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/translations)

set(ZH_TS ${CMAKE_CURRENT_SOURCE_DIR}/translations/zh_CN.ts)
set(EN_TS ${CMAKE_CURRENT_SOURCE_DIR}/translations/en.ts)

set(ZH_QM ${CMAKE_CURRENT_SOURCE_DIR}/res/translations/zh_CN.qm)
set(EN_QM ${CMAKE_CURRENT_SOURCE_DIR}/res/translations/en.qm)

# 查找 lupdate
find_program(LUPDATE_EXECUTABLE
    NAMES lupdate
    PATHS "${Qt6_DIR}/bin"
    "${Qt6_PATH}/bin"
    NO_DEFAULT_PATH
)
if(LUPDATE_EXECUTABLE)
    message(STATUS "Found lupdate: ${LUPDATE_EXECUTABLE}")
    
    add_custom_target(update-translations
        COMMAND ${LUPDATE_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}
            -recursive
            -extensions qml,cpp,h
            -ts ${ZH_TS} ${EN_TS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Extracting translatable strings..."
    )
endif()

# 查找 lrelease
find_program(LRELEASE_EXECUTABLE
    NAMES lrelease
    PATHS 
    "${Qt6_DIR}/bin"
    "${Qt6_PATH}/bin"
    NO_DEFAULT_PATH
)
if(LRELEASE_EXECUTABLE)
    message(STATUS "Found lrelease: ${LRELEASE_EXECUTABLE}")
    
    add_custom_command(
        OUTPUT ${ZH_QM}
        COMMAND ${LRELEASE_EXECUTABLE}
            ${ZH_TS} -qm ${ZH_QM}
        DEPENDS ${ZH_TS}
        COMMENT "Compiling Chinese translations"
    )
    
    add_custom_command(
        OUTPUT ${EN_QM}
        COMMAND ${LRELEASE_EXECUTABLE}
            ${EN_TS} -qm ${EN_QM}
        DEPENDS ${EN_TS}
        COMMENT "Compiling English translations"
    )
    
    add_custom_target(compile-translations ALL
        DEPENDS 
            ${ZH_QM}
            ${EN_QM}
    )
    
    if(TARGET ${PROJECT_NAME})
        add_dependencies(${PROJECT_NAME} compile-translations)
    endif()
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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_OUTPUT)
endif()

add_subdirectory(source/driver)
add_subdirectory(source/control)
add_subdirectory(source/model)

add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    res/res.qrc
)

if(WIN32)
    # 要拷贝的所有文件
    set(RUNTIME_FILES
        "${LIBDATACHANNEL_ROOT}/bin/datachannel.dll"
        "${LIBDATACHANNEL_ROOT}/bin/juice.dll"
        "${OPENSSL_ROOT_DIR}/bin/libcrypto-3-x64.dll"
        "${OPENSSL_ROOT_DIR}/bin/libssl-3-x64.dll"
    )

    foreach(file ${RUNTIME_FILES})
        if(EXISTS ${file})
            add_custom_command(TARGET XFileTransit POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${file}"
                    "$<TARGET_FILE_DIR:XFileTransit>/"
                COMMENT "Copying ${file}"
                VERBATIM
            )
        else()
            message(WARNING "Runtime file not found: ${file}")
        endif()
    endforeach()
endif()

if(LUPDATE_EXECUTABLE)
    add_dependencies(${PROJECT_NAME} update-translations)
endif()

# Windows 特有的资源文件
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/res/logo/logo.rc
    )
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_MANIFEST "${CMAKE_CURRENT_LIST_DIR}/res/logo/app.manifest"
    )
        add_compile_options(/utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32 AND MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:CONSOLE"
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