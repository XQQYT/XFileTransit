cmake_minimum_required(VERSION 3.16)

project(XFileTransit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "D:/Qt6.8/6.8.3/mingw_64")
set(CMAKE_BUILD_TYPE Debug)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt6_add_resources(RESOURCE_FILES "src/res.qrc")

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${RESOURCE_FILES}
)

# Link against Qt5 Widgets
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Quick)
