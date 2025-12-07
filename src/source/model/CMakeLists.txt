set(MODEL_SOURCES
    NetworkInfoModel.cpp
    DeviceListModel.cpp
    ICMPScanner.cpp
    FileIconManager.cpp
    FileListModel.cpp
    ConnectionManager.cpp
    ModelManager.cpp
)

set(MODEL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/DeviceListModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/NetworkInfoModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/ICMPScanner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/FileIconManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/FileListModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/ConnectionManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/model/ModelManager.h
)

qt_add_library(model STATIC
    ${MODEL_SOURCES}
    ${MODEL_HEADERS}
)

target_include_directories(model PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(model
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
)
