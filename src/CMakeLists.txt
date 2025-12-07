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

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        ws2_32
        iphlpapi
    )
endif()