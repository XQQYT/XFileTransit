cmake_minimum_required(VERSION 3.16)

project(XFileTransit LANGUAGES CXX)

# if(WIN32)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
# endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "D:/Qt6.8/6.8.3/mingw_64")
set(CMAKE_BUILD_TYPE Debug)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt6_add_resources(RESOURCE_FILES "src/res/res.qrc")

set(SOURCE     
    src/main.cpp
    src/source/model/FileIconManager.cpp
    src/source/model/FileListModel.cpp
    )

set(HEADER
    src/include/model/FileIconManager.h
    src/include/model/FileListModel.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

add_executable(${PROJECT_NAME}
    ${HEADER}
    ${SOURCE}
    ${RESOURCE_FILES}
)

# if(WIN32)
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         WIN32_EXECUTABLE TRUE
#         LINK_FLAGS "-Wl,-subsystem,windows"
#     )
# endif()

# Link against Qt5 Widgets
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Quick Qt6::Gui Qt6::Widgets)
